from __future__ import annotations

from dataclasses import dataclass


@dataclass(frozen=True)
class ElevationRequirement:
    players: int
    linemate: int
    deraumere: int
    sibur: int
    mendiane: int
    phiras: int
    thystame: int


ELEVATION_REQUIREMENTS: dict[int, ElevationRequirement] = {
    1: ElevationRequirement(1, 1, 0, 0, 0, 0, 0),
    2: ElevationRequirement(2, 1, 1, 1, 0, 0, 0),
    3: ElevationRequirement(2, 2, 0, 1, 0, 2, 0),
    4: ElevationRequirement(4, 1, 1, 2, 0, 1, 0),
    5: ElevationRequirement(4, 1, 2, 1, 3, 0, 0),
    6: ElevationRequirement(6, 1, 2, 3, 0, 1, 0),
    7: ElevationRequirement(6, 2, 2, 2, 2, 2, 1),
}

MAX_LEVEL = 8

RESOURCE_DENSITY: dict[str, float] = {
    "food": 0.5,
    "linemate": 0.3,
    "deraumere": 0.15,
    "sibur": 0.1,
    "mendiane": 0.1,
    "phiras": 0.08,
    "thystame": 0.05,
}
