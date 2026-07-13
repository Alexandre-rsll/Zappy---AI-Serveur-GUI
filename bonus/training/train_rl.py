#!/usr/bin/env python3

from __future__ import annotations

import argparse
import os
import platform
import random
import signal
import subprocess
import sys
import time

_REPO_ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, os.path.join(_REPO_ROOT, "bonus", "ai", "src"))

from zappy_ai_rl.config.roles_config import RolesConfig
from zappy_ai_rl.coordination import broadcast_codec
from zappy_ai_rl.decision import actions, rl_policy, role_selector
from zappy_ai_rl.decision.bot_state import BotState
from zappy_ai_rl.decision.role_selector import get_tree
from zappy_ai_rl.logging_setup import Logger
from zappy_ai_rl.network.command_queue import CommandQueue
from zappy_ai_rl.network.connection import Connection, ConnectionClosed
from zappy_ai_rl.protocol import messages, parser


def _server_binary() -> str:
    base = os.path.join(_REPO_ROOT, "ai", "binary")
    name = "macos" if platform.system() == "Darwin" else "linux"
    path = os.path.join(base, name, "zappy_server")
    if not os.access(path, os.X_OK):
        os.chmod(path, 0o755)
    return path


def _start_server(args) -> subprocess.Popen:
    cmd = [
        _server_binary(),
        "-p", str(args.port),
        "-x", str(args.width),
        "-y", str(args.height),
        "-n", args.team,
        "-c", str(args.bots),
        "-f", str(args.freq),
    ]
    return subprocess.Popen(
        cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL
    )


def _stop_server(proc: subprocess.Popen) -> None:
    proc.send_signal(signal.SIGINT)
    try:
        proc.wait(timeout=2)
    except subprocess.TimeoutExpired:
        proc.kill()
        proc.wait()


def _handshake(conn: Connection, team: str) -> tuple[int, int, int]:
    while not conn.incoming_lines:
        conn.poll(None)
    if conn.incoming_lines.popleft() != "WELCOME":
        raise RuntimeError("expected WELCOME")
    conn.queue_send(team)
    while not conn.incoming_lines:
        conn.poll(None)
    client_num = int(conn.incoming_lines.popleft())
    while not conn.incoming_lines:
        conn.poll(None)
    width, height = conn.incoming_lines.popleft().split()
    return client_num, int(width), int(height)


def _make_bot(args) -> BotState:
    conn = Connection(args.host, args.port)
    client_num, width, height = _handshake(conn, args.team)
    queue = CommandQueue(is_terminal=messages.is_response_terminal)
    bot = BotState(conn, queue, Logger(False), width, height, client_num)
    bot._rl_prev = None
    return bot


def _process_line(bot: BotState, line: str, config: RolesConfig) -> None:
    notification = parser.classify_notification(line)
    if notification is not None:
        if isinstance(notification, parser.Dead):
            bot.alive = False
            return
        if isinstance(notification, parser.Message):
            call = broadcast_codec.decode(notification.text)
            if isinstance(call, broadcast_codec.RecruitCall) and call.level == bot.level:
                bot.team_knowledge.note_recruit_call(
                    bot, call.level, call.count, call.rally, notification.direction
                )
        return

    head = bot.queue.head_command()
    if line == "Elevation underway":
        if head != "Incantation":
            bot.frozen_by_incantation = True
            return
    elif line.startswith("Current level:"):
        if head != "Incantation":
            bot.level = int(line.split(":")[1].strip())
            bot.frozen_by_incantation = False
            bot.last_look = None
            bot.depositing = False
            bot.leader_wait_ticks = 0
            return
    elif line == "ko" and bot.frozen_by_incantation and head != "Incantation":
        bot.frozen_by_incantation = False
        bot.last_look = None
        bot.depositing = False
        bot.leader_wait_ticks = 0
        return

    bot.queue.feed_line(line)


def _rl_decide(bot, policy, config, stats) -> None:
    state = rl_policy.featurize(bot, config)
    snap = rl_policy.snapshot(bot)
    if not policy.is_known(state) or random.random() < policy.epsilon:
        action = role_selector._base_cascade(bot, config)
    else:
        action = policy.best_action(state)
    if bot._rl_prev is not None:
        prev_state, prev_action, prev_snap = bot._rl_prev
        reward = rl_policy.compute_reward(prev_snap, snap)
        if action != prev_action:
            reward -= 0.2
        policy.update(prev_state, prev_action, reward, state)
        if snap["level"] > prev_snap["level"]:
            stats["incantations"] += 1
            stats["max_level"] = max(stats["max_level"], snap["level"])
    bot.role_name = action
    bot._rl_prev = (state, action, snap)
    get_tree(action, config)(bot)


def _rl_terminal(bot, policy) -> None:
    if bot._rl_prev is None:
        return
    prev_state, prev_action, prev_snap = bot._rl_prev
    reward = rl_policy.compute_reward(prev_snap, rl_policy.snapshot(bot))
    key = policy._key(prev_state, prev_action)
    old = policy.q.get(key, 0.0)
    policy.q[key] = old + policy.alpha * (reward - old)
    bot._rl_prev = None


def _step_bot(bot, policy, config, stats) -> None:
    bot.tick_count += 1
    if bot.tick_count % 5 == 0:
        actions.check_inventory(bot)
        return
    if bot.tick_count % 11 == 0:
        actions.check_connect_nbr(bot)
        return
    _rl_decide(bot, policy, config, stats)


def run_episode(policy, config, args) -> dict:
    server = _start_server(args)
    time.sleep(0.4)
    stats = {"max_level": 1, "incantations": 0, "ticks": 0, "survivors": 0}
    bots: list[BotState] = []
    try:
        for _ in range(args.bots):
            bots.append(_make_bot(args))
    except (ConnectionClosed, OSError, RuntimeError) as exc:
        for bot in bots:
            bot.conn.close()
        _stop_server(server)
        raise RuntimeError(f"bot setup failed: {exc}") from exc

    try:
        while any(b.alive for b in bots) and stats["ticks"] < args.ticks:
            progressed = False
            for bot in bots:
                if not bot.alive:
                    continue
                try:
                    bot.conn.poll(0)
                except (ConnectionClosed, OSError):
                    bot.alive = False
                    _rl_terminal(bot, policy)
                    continue
                while bot.conn.incoming_lines:
                    _process_line(bot, bot.conn.incoming_lines.popleft(), config)
                    progressed = True
                    if not bot.alive:
                        _rl_terminal(bot, policy)
                        break
                if bot.alive and bot.queue.in_flight_count() == 0:
                    _step_bot(bot, policy, config, stats)
                    stats["ticks"] += 1
                    progressed = True
            if not progressed:
                time.sleep(0.001)
    finally:
        stats["survivors"] = sum(1 for b in bots if b.alive)
        stats["max_level"] = max(stats["max_level"], *(b.level for b in bots))
        for bot in bots:
            try:
                bot.conn.close()
            except OSError:
                pass
        _stop_server(server)
    policy.decay_epsilon()
    return stats


def main() -> int:
    parser_ = argparse.ArgumentParser(description="Train the bonus RL role policy")
    parser_.add_argument("--episodes", type=int, default=150)
    parser_.add_argument("--bots", type=int, default=6)
    parser_.add_argument("--ticks", type=int, default=4000)
    parser_.add_argument("--port", type=int, default=4350)
    parser_.add_argument("--host", type=str, default="localhost")
    parser_.add_argument("--team", type=str, default="T")
    parser_.add_argument("--width", type=int, default=12)
    parser_.add_argument("--height", type=int, default=12)
    parser_.add_argument("--freq", type=int, default=1000)
    parser_.add_argument("--checkpoint-every", type=int, default=10)
    args = parser_.parse_args()

    policy = rl_policy.RLPolicy.load()
    print(
        f"loaded policy: {policy.state_count()} states, eps={policy.epsilon:.3f}",
        flush=True,
    )

    best_level = 0
    for ep in range(1, args.episodes + 1):
        try:
            stats = run_episode(policy, config := RolesConfig(), args)
        except RuntimeError as exc:
            print(f"episode {ep}: {exc} (retrying)", flush=True)
            time.sleep(0.5)
            continue
        best_level = max(best_level, stats["max_level"])
        print(
            f"ep {ep:3d}  maxL={stats['max_level']}  incant={stats['incantations']:2d}  "
            f"surv={stats['survivors']}/{args.bots}  ticks={stats['ticks']}  "
            f"eps={policy.epsilon:.3f}  qstates={policy.state_count()}  best={best_level}",
            flush=True,
        )
        if ep % args.checkpoint_every == 0:
            policy.save()
            print(f"  checkpoint saved ({policy.state_count()} states)", flush=True)

    policy.save()
    print(f"done. final policy: {policy.state_count()} states -> {rl_policy.weights_path()}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
