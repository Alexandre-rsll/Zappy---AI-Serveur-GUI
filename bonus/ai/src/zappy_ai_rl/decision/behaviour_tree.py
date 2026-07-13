from __future__ import annotations

from enum import Enum
from typing import Callable, TypeVar

Status = Enum("Status", ["SUCCESS", "FAILURE", "RUNNING"])

Bot = TypeVar("Bot")
Node = Callable[[Bot], Status]
Predicate = Callable[[Bot], bool]


def sequence(*children: Node) -> Node:
    def _run(bot: Bot) -> Status:
        for child in children:
            status = child(bot)
            if status is not Status.SUCCESS:
                return status
        return Status.SUCCESS

    return _run


def selector(*children: Node) -> Node:
    def _run(bot: Bot) -> Status:
        for child in children:
            status = child(bot)
            if status is not Status.FAILURE:
                return status
        return Status.FAILURE

    return _run


def condition(predicate: Predicate) -> Node:
    def _run(bot: Bot) -> Status:
        return Status.SUCCESS if predicate(bot) else Status.FAILURE

    return _run
