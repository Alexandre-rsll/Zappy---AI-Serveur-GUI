/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** GUIClient
*/

#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <array>
#include <iostream>
#include <stdexcept>

#include "gui_client/GUIClient.hpp"

namespace gui {

    GUIClient::GUIClient(const GUIConfig &config) : _protocolHandler(_gameState)
    {
        _connectToServer(config);
        _setupPollFds();
        _authenticate();
    }

    GUIClient::~GUIClient()
    {
        stop();
        if (_socketFd >= 0) {
            close(_socketFd);
        }
    }

    void GUIClient::_connectToServer(const GUIConfig &config)
    {
        addrinfo hints{};
        addrinfo *result = nullptr;

        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        const std::string port = std::to_string(config.port);

        if (getaddrinfo(config.machine.c_str(), port.c_str(), &hints,
                        &result) != 0) {
            throw std::runtime_error("Failed to resolve host");
        }

        for (addrinfo *rp = result; rp != nullptr; rp = rp->ai_next) {
            _socketFd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
            if (_socketFd == -1) {
                continue;
            }

            if (connect(_socketFd, rp->ai_addr, rp->ai_addrlen) == 0) {
                break;
            }

            close(_socketFd);
            _socketFd = -1;
        }

        freeaddrinfo(result);

        if (_socketFd == -1) {
            throw std::runtime_error("Connection failed");
        }
    }

    void GUIClient::_setupPollFds()
    {
        pollfd socketFd{};

        socketFd.fd = _socketFd;
        socketFd.events = POLLIN;
        socketFd.revents = 0;

        _pollFds.clear();
        _pollFds.push_back(socketFd);
    }

    void GUIClient::_authenticate()
    {
        const std::string welcome = this->_readLineBlocking();

        if (welcome != "WELCOME") {
            throw std::runtime_error("Invalid server welcome message");
        }

        this->_sendLine("GRAPHIC");
    }

    void GUIClient::startProtocolThread()
    {
        if (_running.load()) {
            return;
        }

        _running.store(true);
        _protocolThread = std::thread(&GUIClient::_protocolLoop, this);
    }

    void GUIClient::stop()
    {
        _running.store(false);

        if (_protocolThread.joinable()) {
            _protocolThread.join();
        }
    }

    bool GUIClient::isRunning() const
    {
        return _running.load();
    }

    GameState GUIClient::getGameStateSnapshot() const
    {
        const std::lock_guard<std::mutex> lock(_stateMutex);

        return _gameState;
    }

    void GUIClient::sendProtocolCommand(const std::string &command) const
    {
        _sendLine(command);
    }

    void GUIClient::_protocolLoop()
    {
        while (_running.load()) {
            const int pollResult =
                poll(_pollFds.data(), static_cast<nfds_t>(_pollFds.size()),
                     gui::pollTimeoutMs);

            if (pollResult < 0) {
                _running.store(false);
                return;
            }
            if (pollResult == 0) {
                continue;
            }
            const pollfd &socketFd = _pollFds.at(0);
            if ((socketFd.revents & POLLIN) != 0) {
                this->_handleServerData();
            }
            if ((socketFd.revents & (POLLHUP | POLLERR)) != 0) {
                _running.store(false);
            }
        }
    }

    bool GUIClient::_receiveData()
    {
        std::array<char, readBufferSize> buffer{};
        ssize_t received = 0;

        received = recv(_socketFd, buffer.data(), buffer.size(), 0);

        if (received > 0) {
            _buffer.append(buffer.data(), static_cast<std::size_t>(received));
            return true;
        }

        return false;
    }

    bool GUIClient::_extractLine(std::string &line)
    {
        const std::size_t pos = _buffer.find('\n');

        if (pos == std::string::npos) {
            return false;
        }

        line = _buffer.substr(0, pos);
        _buffer.erase(0, pos + 1);

        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        return true;
    }

    std::string GUIClient::_readLineBlocking()
    {
        std::string line;

        while (!_extractLine(line)) {
            if (!_receiveData()) {
                throw std::runtime_error("Server disconnected");
            }
        }

        return line;
    }

    void GUIClient::_handleServerData()
    {
        if (!_receiveData()) {
            _running.store(false);
            return;
        }

        std::string line;

        while (_extractLine(line)) {
            if (!line.empty()) {
                _handleLine(line);
            }
        }
    }

    void GUIClient::_handleLine(const std::string &line)
    {
        const std::lock_guard<std::mutex> lock(_stateMutex);

        try {
            _protocolHandler.handleLine(line);
        } catch (const std::exception &exception) {
            _gameState.lastServerMessage = exception.what();
        }
    }

    void GUIClient::_sendLine(const std::string &line) const
    {
        const std::lock_guard<std::mutex> lock(_sendMutex);
        const std::string message = line + "\n";

        if (send(_socketFd, message.c_str(), message.size(), 0) < 0) {
            throw std::runtime_error("Failed to send data");
        }
    }
} // namespace gui
