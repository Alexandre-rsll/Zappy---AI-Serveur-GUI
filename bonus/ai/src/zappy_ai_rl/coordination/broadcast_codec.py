from __future__ import annotations

from dataclasses import dataclass

_RECRUIT_PREFIX = "INCANT"


@dataclass(frozen=True)
class RecruitCall:
    level: int
    count: int
    rally: bool = False


def encode_recruit(level: int, count: int, rally: bool = False) -> str:
    return f"{_RECRUIT_PREFIX}:{level}:{count}:{int(rally)}"


def decode(text: str) -> RecruitCall | None:
    prefix, _, payload = text.partition(":")
    if prefix != _RECRUIT_PREFIX:
        return None
    parts = payload.split(":")
    if len(parts) < 2:
        return None
    try:
        level = int(parts[0])
        count = int(parts[1])
    except ValueError:
        return None
    rally = len(parts) >= 3 and parts[2] == "1"
    return RecruitCall(level=level, count=count, rally=rally)
