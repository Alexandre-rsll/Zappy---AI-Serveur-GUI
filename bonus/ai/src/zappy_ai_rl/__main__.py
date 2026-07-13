from __future__ import annotations

import os
import signal
import sys

from zappy_ai_rl.cli import parse_args
from zappy_ai_rl.config.roles_config import RolesConfig
from zappy_ai_rl.coordination import broadcast_codec
from zappy_ai_rl.decision import actions
from zappy_ai_rl.decision.bot_state import BotState
from zappy_ai_rl.decision.role_selector import compute_role_name, get_tree
from zappy_ai_rl.logging_setup import Logger
from zappy_ai_rl.network.command_queue import CommandQueue
from zappy_ai_rl.network.connection import Connection, ConnectionClosed
from zappy_ai_rl.protocol import messages, parser

_INVENTORY_POLL_INTERVAL = 5
_CONNECT_NBR_POLL_INTERVAL = 11

_READY_FD_ENV = "ZAPPY_AI_READY_FD"


class ShutdownRequested(Exception):
    pass


def _on_sigterm(_signum: int, _frame: object) -> None:
    raise ShutdownRequested()


def _signal_ready() -> None:
    fd_str = os.environ.get(_READY_FD_ENV)
    if fd_str is None:
        return
    try:
        os.write(int(fd_str), b"READY\n")
        os.close(int(fd_str))
    except OSError:
        pass


def _do_handshake(
    conn: Connection, team_name: str, log: Logger
) -> tuple[int, int, int]:
    while not conn.incoming_lines:
        conn.poll(None)
    welcome = conn.incoming_lines.popleft()
    if welcome != "WELCOME":
        raise RuntimeError(f"expected WELCOME, got {welcome!r}")
    log.debug(f"<- {welcome!r}")

    conn.queue_send(team_name)
    log.debug(f"-> {team_name!r}")

    while not conn.incoming_lines:
        conn.poll(None)
    client_num_line = conn.incoming_lines.popleft()
    if not client_num_line.lstrip("-").isdigit():
        raise RuntimeError(
            f"team {team_name!r} rejected (no free slot?): {client_num_line!r}"
        )
    client_num = int(client_num_line)

    while not conn.incoming_lines:
        conn.poll(None)
    width_str, height_str = conn.incoming_lines.popleft().split()

    return client_num, int(width_str), int(height_str)


def _tick(bot: BotState, config: RolesConfig) -> None:
    if bot.queue.in_flight_count() > 0:
        return
    bot.tick_count += 1
    if bot.tick_count % _INVENTORY_POLL_INTERVAL == 0:
        actions.check_inventory(bot)
        return
    if bot.tick_count % _CONNECT_NBR_POLL_INTERVAL == 0:
        actions.check_connect_nbr(bot)
        return
    role_name = compute_role_name(bot, config)
    if role_name != bot.role_name:
        bot.log.debug(f"role: {bot.role_name} -> {role_name}")
        bot.role_name = role_name
    get_tree(role_name, config)(bot)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    log = Logger(args.debug)
    signal.signal(signal.SIGTERM, _on_sigterm)

    conn = Connection(args.host, args.port)
    try:
        client_num, width, height = _do_handshake(conn, args.name, log)
    except (ConnectionClosed, OSError, RuntimeError) as exc:
        print(f"zappy_ai: connection failed: {exc}", file=sys.stderr)
        conn.close()
        return 1
    except (KeyboardInterrupt, ShutdownRequested):
        log.debug("shutdown requested during handshake")
        conn.close()
        return 1
    log.debug(
        f"handshake complete: width={width} height={height} "
        f"client_num={client_num}"
    )
    _signal_ready()

    queue = CommandQueue(is_terminal=messages.is_response_terminal)
    bot = BotState(conn, queue, log, width, height, client_num)
    config = RolesConfig()

    _tick(bot, config)
    log.debug("entering decision loop (Ctrl+C to stop)")

    try:
        while True:
            while not conn.incoming_lines:
                conn.poll(None)
            line = conn.incoming_lines.popleft()

            notification = parser.classify_notification(line)
            if notification is not None:
                log.debug(f"<- notification {notification!r}")
                if isinstance(notification, parser.Dead):
                    bot.alive = False
                    break
                if isinstance(notification, parser.Message):
                    call = broadcast_codec.decode(notification.text)
                    if isinstance(call, broadcast_codec.RecruitCall):
                        if call.level == bot.level:
                            bot.team_knowledge.note_recruit_call(
                                bot,
                                call.level,
                                call.count,
                                call.rally,
                                notification.direction,
                            )
                continue

            head = queue.head_command()
            if line == "Elevation underway":
                if head != "Incantation":
                    bot.frozen_by_incantation = True
                    log.debug("<- passive incantation start (not ours)")
                    continue
            elif line.startswith("Current level:"):
                if head != "Incantation":
                    bot.level = int(line.split(":")[1].strip())
                    bot.frozen_by_incantation = False
                    bot.last_look = None
                    bot.depositing = False
                    bot.leader_wait_ticks = 0
                    log.debug(f"<- passive incantation -> level {bot.level}")
                    continue
            elif (
                line == "ko"
                and bot.frozen_by_incantation
                and head != "Incantation"
            ):
                bot.frozen_by_incantation = False
                bot.last_look = None
                bot.depositing = False
                bot.leader_wait_ticks = 0
                log.debug("<- passive incantation failed (not ours)")
                continue

            result = queue.feed_line(line)
            if result is None:
                log.debug(f"<- partial response line {line!r}")
                continue
            command, lines = result
            log.debug(
                f"<- response to {command!r}: {lines!r} "
                f"(level={bot.level} role={bot.role_name})"
            )
            _tick(bot, config)
    except (KeyboardInterrupt, ShutdownRequested):
        log.debug("shutdown requested")
    except (ConnectionClosed, OSError) as exc:
        log.debug(f"connection lost: {exc}")
    finally:
        conn.close()
    return 0 if bot.alive else 1


if __name__ == "__main__":
    raise SystemExit(main())
