from __future__ import annotations

from dataclasses import dataclass
from zappy_ai_rl.protocol.parser import strip_brackets


@dataclass(frozen=True)
class Tile:
    forward: int
    lateral: int
    objects: list[str]


def offsets_for_level(level: int) -> list[tuple[int, int]]:
    offsets = [(0, 0)]
    for row in range(1, level + 1):
        offsets.extend((row, lateral) for lateral in range(-row, row + 1))
    return offsets


def parse_look(raw: str, level: int) -> list[Tile]:
    cells = [cell.strip() for cell in strip_brackets(raw).split(",")]
    offsets = offsets_for_level(level)
    if len(cells) != len(offsets):
        raise ValueError(
            f"expected {len(offsets)} tiles for level {level}, "
            f"got {len(cells)}: {raw!r}"
        )
    return [
        Tile(forward=forward, lateral=lateral, objects=cell.split() if cell else [])
        for (forward, lateral), cell in zip(offsets, cells)
    ]
