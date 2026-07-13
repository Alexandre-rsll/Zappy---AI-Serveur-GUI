from __future__ import annotations

from typing import Callable

from zappy_ai_rl.coordination.team_knowledge import TeamKnowledge
from zappy_ai_rl.logging_setup import Logger
from zappy_ai_rl.network.command_queue import CommandQueue
from zappy_ai_rl.network.connection import Connection
from zappy_ai_rl.world.inventory import Inventory
from zappy_ai_rl.world.mental_map import MentalMap
from zappy_ai_rl.world.vision import Tile


class BotState:
    def __init__(
        self,
        conn: Connection,
        queue: CommandQueue,
        log: Logger,
        width: int,
        height: int,
        client_num: int,
    ) -> None:
        self.conn = conn
        self.queue = queue
        self.log = log
        self.level = 1
        self.inventory = Inventory(food=10)
        self.mental_map = MentalMap(width=width, height=height)
        self.last_look: list[Tile] | None = None
        self.alive = True
        self.role_name = "COLLECTOR"
        self.tick_count = 0
        self.nav_force_forward = False
        self.depositing = False
        self.deposit_position: tuple[int, int] | None = None
        self.free_slots = client_num
        self.forks_done = 0
        self.seek_position: tuple[int, int] | None = None
        self.leader_wait_ticks = 0
        self.leader_cooldown_until = 0
        self.team_knowledge = TeamKnowledge()
        self.frozen_by_incantation = False

    def send(
        self,
        command: str,
        on_complete: Callable[[list[str]], None] | None = None,
    ) -> bool:
        if not self.queue.has_room():
            return False
        self.conn.queue_send(command)
        self.queue.note_sent(command, on_complete=on_complete)
        self.log.debug(f"-> {command!r}")
        return True
