from __future__ import annotations

from zappy_ai_rl.config.roles_config import ForkerConfig
from zappy_ai_rl.decision import actions
from zappy_ai_rl.decision.behaviour_tree import Node


def is_triggered(bot, config: ForkerConfig) -> bool:
    if bot.forks_done >= config.max_forks:
        return False
    return bot.free_slots < config.target_free_slots


def build_tree(_config: ForkerConfig) -> Node:
    return actions.fork
