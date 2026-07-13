from __future__ import annotations

import argparse
from dataclasses import dataclass


@dataclass(frozen=True)
class Args:
    port: int
    name: str
    host: str
    debug: bool


def parse_args(argv: list[str] | None = None) -> Args:
    parser = argparse.ArgumentParser(
        prog="zappy_ai",
        usage="./zappy_ai -p port -n name -h machine",
        add_help=False,
    )
    parser.add_argument(
        "--help", action="help", help="show this help message and exit"
    )
    parser.add_argument(
        "-p", dest="port", type=int, required=True, help="port number"
    )
    parser.add_argument(
        "-n", dest="name", type=str, required=True, help="name of the team"
    )
    parser.add_argument(
        "-h",
        dest="host",
        type=str,
        default="localhost",
        help="name of the machine; localhost by default",
    )
    parser.add_argument(
        "--debug",
        dest="debug",
        action="store_true",
        help="enable debug logging on stderr",
    )
    namespace = parser.parse_args(argv)
    return Args(
        port=namespace.port,
        name=namespace.name,
        host=namespace.host,
        debug=namespace.debug,
    )
