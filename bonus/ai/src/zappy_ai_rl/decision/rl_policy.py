from __future__ import annotations

import json
import os
import random

ACTIONS: tuple[str, ...] = ("SURVIVOR", "FORKER", "LEADER", "COLLECTOR")

_WEIGHTS_PATH = os.path.join(os.path.dirname(__file__), "rl_weights.json")


def weights_path() -> str:
    return _WEIGHTS_PATH


def _survival_bucket(time_remaining: int) -> int:
    if time_remaining < 126:
        return 0
    if time_remaining < 630:
        return 1
    if time_remaining < 1260:
        return 2
    return 3


def _free_slots_bucket(free_slots: int) -> int:
    if free_slots <= 0:
        return 0
    if free_slots == 1:
        return 1
    return 2


def featurize(bot, config) -> tuple:
    from zappy_ai_rl.decision.roles import collector

    survival = _survival_bucket(bot.inventory.survival_time_remaining())
    level = min(max(bot.level, 1), 8)
    slots = _free_slots_bucket(bot.free_slots)
    stones_ready = collector.stones_ready(bot)
    recruit_known = (
        bot.team_knowledge.recruit_heading(
            bot.level, bot.tick_count, config.collector.recruit_call_max_age_ticks
        )
        is not None
    )
    return (survival, level, slots, int(stones_ready), int(recruit_known))


class RLPolicy:
    def __init__(
        self,
        alpha: float = 0.3,
        gamma: float = 0.95,
        epsilon: float = 0.5,
        epsilon_min: float = 0.05,
        epsilon_decay: float = 0.985,
    ) -> None:
        self.q: dict[str, float] = {}
        self.alpha = alpha
        self.gamma = gamma
        self.epsilon = epsilon
        self.epsilon_min = epsilon_min
        self.epsilon_decay = epsilon_decay

    @staticmethod
    def _key(state: tuple, action: str) -> str:
        return json.dumps([list(state), action])

    def q_value(self, state: tuple, action: str) -> float:
        return self.q.get(self._key(state, action), 0.0)

    def is_known(self, state: tuple) -> bool:
        return any(self._key(state, a) in self.q for a in ACTIONS)

    def best_action(self, state: tuple) -> str:
        return max(ACTIONS, key=lambda a: (self.q_value(state, a), random.random()))

    def select_action(self, state: tuple, explore: bool) -> str:
        if explore and random.random() < self.epsilon:
            return random.choice(ACTIONS)
        return self.best_action(state)

    def update(
        self, state: tuple, action: str, reward: float, next_state: tuple
    ) -> None:
        best_next = max(self.q_value(next_state, a) for a in ACTIONS)
        key = self._key(state, action)
        old = self.q.get(key, 0.0)
        self.q[key] = old + self.alpha * (reward + self.gamma * best_next - old)

    def decay_epsilon(self) -> None:
        self.epsilon = max(self.epsilon_min, self.epsilon * self.epsilon_decay)

    def save(self, path: str | None = None) -> None:
        path = path or _WEIGHTS_PATH
        tmp = path + ".tmp"
        with open(tmp, "w", encoding="utf-8") as handle:
            json.dump(
                {"q": self.q, "epsilon": self.epsilon, "states": self.state_count()},
                handle,
            )
        os.replace(tmp, path)

    def state_count(self) -> int:
        return len({k.rsplit(",", 1)[0] for k in self.q})

    @classmethod
    def load(cls, path: str | None = None) -> "RLPolicy":
        path = path or _WEIGHTS_PATH
        policy = cls()
        if os.path.exists(path):
            with open(path, encoding="utf-8") as handle:
                data = json.load(handle)
            policy.q = data.get("q", {})
            policy.epsilon = data.get("epsilon", policy.epsilon)
        return policy


def compute_reward(prev: dict, curr: dict) -> float:
    reward = -0.02
    if curr["level"] > prev["level"]:
        reward += 10.0 * curr["level"]
    if prev["alive"] and not curr["alive"]:
        reward -= 50.0
    if curr["free_slots"] > prev["free_slots"]:
        reward += 0.5
    return reward


def snapshot(bot) -> dict:
    return {"level": bot.level, "alive": bot.alive, "free_slots": bot.free_slots}
