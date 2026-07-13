/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** Client
*/

#include <unistd.h>
#include <sys/socket.h>
#include <array>
#include <utility>

#include "network_server/Client.hpp"

namespace server {
    int Client::_nextId = 1;

    Client::Client(const int fd) : _fd(fd), _id(_nextId++)
    {}

    Client::~Client()
    {
        if (_fd <= 0) {
            close(_fd);
        }
    }

    int Client::fd() const
    {
        return _fd;
    }

    int Client::id() const
    {
        return _id;
    }

    ClientState Client::state() const
    {
        return _state;
    }

    const std::string &Client::teamName() const
    {
        return _teamName;
    }

    std::shared_ptr<Trantorian> Client::player()
    {
        return _player;
    }

    std::shared_ptr<const Trantorian> Client::player() const
    {
        return _player;
    }

    std::size_t Client::commandCount() const
    {
        return _commandQueue.size();
    }

    void Client::setState(const ClientState state)
    {
        _state = state;
    }

    void Client::setTeamName(const std::string &teamName)
    {
        _teamName = teamName;
    }

    void Client::setPlayer(std::shared_ptr<Trantorian> player)
    {
        _player = std::move(player);
    }

    bool Client::readSocket()
    {
        std::array<char, maxClientInput> buffer{};
        const ssize_t bytesRead = read(_fd, buffer.data(), buffer.size());
        if (bytesRead <= 0) {
            return false;
        }
        _readBuffer.append(buffer.data(), bytesRead);
        return true;
    }

    bool Client::readLine(std::string &out)
    {
        const auto pos = _readBuffer.find('\n');
        if (pos == std::string::npos) {
            return false;
        }

        out = _readBuffer.substr(0, pos);
        if (!out.empty() && out.back() == '\r') {
            out.pop_back();
        }
        _readBuffer.erase(0, pos + 1);
        return true;
    }

    void Client::pushResponse(const std::string &msg)
    {
        _writeBuffer += msg;
    }

    bool Client::sendWriteBuffer()
    {
        if (_writeBuffer.empty()) {
            return true;
        }

        const ssize_t bytesSent =
            send(_fd, _writeBuffer.c_str(), _writeBuffer.size(), MSG_NOSIGNAL);
        if (bytesSent < 0) {
            return false;
        }

        _writeBuffer.erase(0, bytesSent);
        return true;
    }

    bool Client::hasDataToWrite() const
    {
        return !_writeBuffer.empty();
    }

    bool Client::pushCommand(const std::string &raw)
    {
        if (_commandQueue.size() >= maxPendingCommands) {
            return false;
        }
        _commandQueue.push({.raw = raw, .readyAt = {}, .executing = false});
        return true;
    }

    bool Client::hasCommand() const
    {
        return !_commandQueue.empty();
    }

    PendingCommand &Client::frontCommand()
    {
        return _commandQueue.front();
    }

    void Client::popCommand()
    {
        if (!_commandQueue.empty()) {
            _commandQueue.pop();
        }
    }

    void Client::startCommand(const double duration)
    {
        if (_commandQueue.empty()) {
            return;
        }
        auto &cmd = _commandQueue.front();
        cmd.executing = true;
        cmd.readyAt =
            std::chrono::steady_clock::now() +
            std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                std::chrono::duration<double>(duration));
    }
} // namespace server
