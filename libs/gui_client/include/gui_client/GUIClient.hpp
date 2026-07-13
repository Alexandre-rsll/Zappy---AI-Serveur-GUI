/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** GUIClient
*/

#ifndef GUICLIENT_HPP
#define GUICLIENT_HPP

#include <atomic>
#include <mutex>
#include <poll.h>
#include <string>
#include <thread>
#include <vector>

#include "gui_client/GUIConfig.hpp"
#include "gui_protocol/GameState.hpp"
#include "gui_protocol/GUIProtocolHandler.hpp"

namespace gui {

    constexpr std::size_t readBufferSize = 4096;
    constexpr int pollTimeoutMs = 10;

    class GUIClient {
    public:
        explicit GUIClient(const GUIConfig &config);
        ~GUIClient();

        void startProtocolThread();
        void stop();

        [[nodiscard]] bool isRunning() const;
        [[nodiscard]] GameState getGameStateSnapshot() const;

        void sendProtocolCommand(const std::string &command) const;

    private:
        int _socketFd = -1;
        std::atomic_bool _running = false;
        std::thread _protocolThread{};
        std::vector<pollfd> _pollFds{};
        std::string _buffer{};

        mutable std::mutex _stateMutex{};
        mutable std::mutex _sendMutex{};

        GameState _gameState{};
        GUIProtocolHandler _protocolHandler;

        void _connectToServer(const GUIConfig &config);
        void _setupPollFds();

        void _authenticate();
        void _sendLine(const std::string &line) const;

        void _handleServerData();
        void _handleLine(const std::string &line);

        void _protocolLoop();
        bool _receiveData();
        bool _extractLine(std::string &line);
        std::string _readLineBlocking();
    };

} // namespace gui

#endif /* !GUICLIENT_HPP */