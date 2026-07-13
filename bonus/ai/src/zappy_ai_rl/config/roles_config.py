from __future__ import annotations

from dataclasses import dataclass


@dataclass(frozen=True)
class SurvivorConfig:
    critical_threshold: int = 700


@dataclass(frozen=True)
class ForkerConfig:
    target_free_slots: int = 1
    max_forks: int = 2


@dataclass(frozen=True)
class LeaderConfig:
    patience_ticks: int = 40
    broadcast_interval_ticks: int = 2
    cooldown_ticks: int = 15
    rival_call_max_age_ticks: int = 8
    yield_margin: int = 1
    rally_min_players: int = 4


@dataclass(frozen=True)
class CollectorConfig:
    recruit_call_max_age_ticks: int = 10


@dataclass(frozen=True)
class RolesConfig:
    survivor: SurvivorConfig = SurvivorConfig()
    forker: ForkerConfig = ForkerConfig()
    leader: LeaderConfig = LeaderConfig()
    collector: CollectorConfig = CollectorConfig()
