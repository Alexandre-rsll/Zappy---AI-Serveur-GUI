/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** ZappyServerCore
*/

#ifndef ZAPPY_ZAPPYSERVERCORE_HPP
#define ZAPPY_ZAPPYSERVERCORE_HPP

#include "cli/CLIManager.hpp"
#include "network_server/Server.hpp"

namespace zappyServer {
    class ZappyServerCore {
    public:
        explicit ZappyServerCore(int ac, char **av);
        ~ZappyServerCore() = default;

        server::ServerConfig getServerConfig() const;

    private:
        void _registerCLIOptions();
        std::size_t _getRequiredSizeT(const std::string &key) const;
        const std::string &_getRequiredValue(const std::string &key) const;

        cli::CLIManager _manager;
    };
} // namespace zappyServer

#endif // ZAPPY_ZAPPYSERVERCORE_HPP
