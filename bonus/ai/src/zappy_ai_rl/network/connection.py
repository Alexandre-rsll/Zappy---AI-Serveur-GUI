from __future__ import annotations

import errno
import os
import selectors
import socket
from collections import deque


class ConnectionClosed(Exception):
    pass


class Connection:
    def __init__(self, host: str, port: int) -> None:
        self._selector = selectors.DefaultSelector()
        self._sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._sock.setblocking(False)
        self._read_buffer = b""
        self._write_buffer = b""
        self._connected = False
        self.incoming_lines: deque[str] = deque()
        self.closed = False

        connect_error = self._sock.connect_ex((host, port))
        if connect_error not in (0, errno.EINPROGRESS, errno.EWOULDBLOCK):
            raise OSError(connect_error, os.strerror(connect_error))
        self._selector.register(
            self._sock, selectors.EVENT_READ | selectors.EVENT_WRITE
        )

    def queue_send(self, text: str) -> None:
        had_pending = bool(self._write_buffer)
        self._write_buffer += text.encode("utf-8") + b"\n"
        if not had_pending and self._connected:
            self._update_registration()

    def poll(self, timeout: float | None) -> None:
        for key, mask in self._selector.select(timeout):
            if mask & selectors.EVENT_WRITE:
                self._on_writable()
            if mask & selectors.EVENT_READ:
                self._on_readable()

    def close(self) -> None:
        if self.closed:
            return
        self._selector.unregister(self._sock)
        self._selector.close()
        self._sock.close()
        self.closed = True

    def _on_writable(self) -> None:
        if not self._connected:
            connect_error = self._sock.getsockopt(
                socket.SOL_SOCKET, socket.SO_ERROR
            )
            if connect_error != 0:
                raise OSError(connect_error, os.strerror(connect_error))
            self._connected = True
        if self._write_buffer:
            try:
                sent = self._sock.send(self._write_buffer)
            except BlockingIOError:
                return
            self._write_buffer = self._write_buffer[sent:]
        self._update_registration()

    def _on_readable(self) -> None:
        try:
            chunk = self._sock.recv(4096)
        except BlockingIOError:
            return
        if not chunk:
            self.closed = True
            raise ConnectionClosed("server closed the connection")
        self._read_buffer += chunk
        while b"\n" in self._read_buffer:
            raw_line, self._read_buffer = self._read_buffer.split(b"\n", 1)
            self.incoming_lines.append(raw_line.decode("utf-8"))

    def _update_registration(self) -> None:
        events = selectors.EVENT_READ
        if self._write_buffer or not self._connected:
            events |= selectors.EVENT_WRITE
        self._selector.modify(self._sock, events)
