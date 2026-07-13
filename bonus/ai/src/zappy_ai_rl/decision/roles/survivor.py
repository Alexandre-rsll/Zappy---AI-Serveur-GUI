from __future__ import annotations

from zappy_ai_rl.config.roles_config import SurvivorConfig
from zappy_ai_rl.decision import actions
from zappy_ai_rl.decision.behaviour_tree import Node, condition, selector, sequence
from zappy_ai_rl.decision.navigation import seek_resource


def is_triggered(bot, config: SurvivorConfig) -> bool:
    return bot.inventory.survival_time_remaining() < config.critical_threshold


def _has_food_here(bot) -> bool:
    return bot.last_look is not None and "food" in bot.last_look[0].objects


def _seek_food(bot):
    return seek_resource(bot, "food")


def build_tree(_config: SurvivorConfig) -> Node:
    return selector(
        sequence(condition(_has_food_here), actions.take("food")),
        _seek_food,
    )
