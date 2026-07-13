#!/usr/bin/env python3

from __future__ import annotations

import argparse
import os
import statistics
import sys
import time

_REPO_ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, os.path.join(_REPO_ROOT, "bonus", "ai", "src"))

from zappy_ai_rl.config.roles_config import RolesConfig
from zappy_ai_rl.decision import actions, rl_policy
from zappy_ai_rl.decision import role_selector
from zappy_ai_rl.network.connection import ConnectionClosed

import train_rl


def _select_base(bot, config):
    return role_selector._base_cascade(bot, config)


def _make_rl_selector():
    policy = rl_policy.RLPolicy.load()

    def _select(bot, config):
        state = rl_policy.featurize(bot, config)
        if not policy.is_known(state):
            return role_selector._base_cascade(bot, config)
        return policy.select_action(state, explore=False)

    return _select, policy


def _step_bot(bot, select, config, stats):
    bot.tick_count += 1
    if bot.tick_count % 5 == 0:
        actions.check_inventory(bot)
        return
    if bot.tick_count % 11 == 0:
        actions.check_connect_nbr(bot)
        return
    if bot.level > stats["max_level"]:
        stats["max_level"] = bot.level
    role = select(bot, config)
    bot.role_name = role
    role_selector.get_tree(role, config)(bot)


def run_episode(select, config, args) -> dict:
    server = train_rl._start_server(args)
    time.sleep(0.4)
    stats = {"max_level": 1, "incantations": 0, "ticks": 0, "survivors": 0}
    levels_before = {}
    bots = []
    try:
        for _ in range(args.bots):
            bots.append(train_rl._make_bot(args))
    except (ConnectionClosed, OSError, RuntimeError):
        for bot in bots:
            bot.conn.close()
        train_rl._stop_server(server)
        raise
    for i, bot in enumerate(bots):
        levels_before[i] = bot.level
    try:
        while any(b.alive for b in bots) and stats["ticks"] < args.ticks:
            progressed = False
            for i, bot in enumerate(bots):
                if not bot.alive:
                    continue
                try:
                    bot.conn.poll(0)
                except (ConnectionClosed, OSError):
                    bot.alive = False
                    continue
                while bot.conn.incoming_lines:
                    train_rl._process_line(bot, bot.conn.incoming_lines.popleft(), config)
                    progressed = True
                    if not bot.alive:
                        break
                if bot.level > levels_before[i]:
                    stats["incantations"] += 1
                    levels_before[i] = bot.level
                if bot.alive and bot.queue.in_flight_count() == 0:
                    _step_bot(bot, select, config, stats)
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
        train_rl._stop_server(server)
    return stats


def _summary(name, runs):
    levels = [r["max_level"] for r in runs]
    incs = [r["incantations"] for r in runs]
    survs = [r["survivors"] for r in runs]
    print(
        f"{name:5s}  maxL avg={statistics.mean(levels):.2f} max={max(levels)}  "
        f"incant avg={statistics.mean(incs):.1f}  survivors avg={statistics.mean(survs):.1f}"
    )


def main() -> int:
    parser = argparse.ArgumentParser(description="Compare base vs RL policy")
    parser.add_argument("--runs", type=int, default=5)
    parser.add_argument("--bots", type=int, default=6)
    parser.add_argument("--ticks", type=int, default=6000)
    parser.add_argument("--port", type=int, default=4360)
    parser.add_argument("--host", type=str, default="localhost")
    parser.add_argument("--team", type=str, default="T")
    parser.add_argument("--width", type=int, default=12)
    parser.add_argument("--height", type=int, default=12)
    parser.add_argument("--freq", type=int, default=1000)
    args = parser.parse_args()

    config = RolesConfig()
    rl_select, policy = _make_rl_selector()
    print(f"RL policy: {policy.state_count()} states\n")

    base_runs, rl_runs = [], []
    for i in range(args.runs):
        base_runs.append(run_episode(_select_base, config, args))
        print(f"  base run {i+1}: {base_runs[-1]}", flush=True)
    for i in range(args.runs):
        rl_runs.append(run_episode(rl_select, config, args))
        print(f"  rl   run {i+1}: {rl_runs[-1]}", flush=True)

    print()
    _summary("base", base_runs)
    _summary("rl", rl_runs)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
