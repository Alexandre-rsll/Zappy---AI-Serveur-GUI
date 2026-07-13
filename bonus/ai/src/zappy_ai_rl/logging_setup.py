from __future__ import annotations

import os
import sys
import time


class Logger:
    def __init__(self, enabled: bool) -> None:
        self._enabled = enabled or os.environ.get("ZAPPY_AI_DEBUG") == "1"

    def debug(self, message: str) -> None:
        if self._enabled:
            print(f"[{time.monotonic():.3f}] {message}", file=sys.stderr)
