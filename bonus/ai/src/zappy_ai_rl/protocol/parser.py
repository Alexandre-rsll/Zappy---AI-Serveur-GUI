from __future__ import annotations

from dataclasses import dataclass
from typing import Union


@dataclass(frozen=True)
class Dead:
    pass


@dataclass(frozen=True)
class Message:
    direction: int
    text: str


@dataclass(frozen=True)
class Eject:
    direction: int


Notification = Union[Dead, Message, Eject]


def classify_notification(line: str) -> Notification | None:
    if line == "dead":
        return Dead()
    if line.startswith("message "):
        direction_str, _, text = line[len("message "):].partition(",")
        return Message(direction=int(direction_str), text=text.lstrip(" "))
    if line.startswith("eject: "):
        return Eject(direction=int(line[len("eject: "):]))
    return None


def strip_brackets(raw: str) -> str:
    inner = raw.strip()
    if inner.startswith("["):
        inner = inner[1:]
    if inner.endswith("]"):
        inner = inner[:-1]
    return inner.strip()
