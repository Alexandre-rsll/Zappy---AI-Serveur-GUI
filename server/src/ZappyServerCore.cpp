/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** ZappyServerCore
*/

#include <iostream>

#include "ZappyServerCore.hpp"
#include "network_server/Server.hpp"

namespace zappyServer {
    ZappyServerCore::ZappyServerCore(const int ac, char **av)
    {
        const std::vector<std::string> params(av + 1, av + ac);
        _registerCLIOptions();
        if (!_manager.parse(params)) {
            throw cli::CLIManagerException(
                "Invalid Zappy server configuration");
        }
    }

    server::ServerConfig ZappyServerCore::getServerConfig() const
    {
        server::ServerConfig config{};

        config.port = _getRequiredSizeT("p");
        config.width = _getRequiredSizeT("x");
        config.height = _getRequiredSizeT("y");
        config.clientsNb = _getRequiredSizeT("c");
        config.freq = _getRequiredSizeT("f");

        if (_manager.getConfig("n").isDefined) {
            config.teamNames = _manager.getConfig("n").values;
        }
        return config;
    }

    std::size_t ZappyServerCore::_getRequiredSizeT(const std::string &key) const
    {
        return std::stoul(_getRequiredValue(key));
    }

    const std::string &
    ZappyServerCore::_getRequiredValue(const std::string &key) const
    {
        const auto &config = _manager.getConfig(key);

        if (!config.value.has_value()) {
            throw std::runtime_error("Missing value for option: -" + key);
        }
        return config.value.value();
    }

    void ZappyServerCore::_registerCLIOptions()
    {
        _manager.registerConfigOption(
            cli::CLIOptionBuilder()
                .setKey("p")
                .setDescription("port")
                .setIsOptional(true)
                .setHasValue(true)
                .setHasMultipleValues(false)
                .setDefaultValue(std::to_string(server::defaultPort))
                .build());

        _manager.registerConfigOption(
            cli::CLIOptionBuilder()
                .setKey("x")
                .setDescription("width")
                .setIsOptional(true)
                .setHasValue(true)
                .setHasMultipleValues(false)
                .setDefaultValue(std::to_string(server::defaultWidth))
                .build());

        _manager.registerConfigOption(
            cli::CLIOptionBuilder()
                .setKey("y")
                .setDescription("height")
                .setIsOptional(true)
                .setHasValue(true)
                .setHasMultipleValues(false)
                .setDefaultValue(std::to_string(server::defaultHeight))
                .build());

        _manager.registerConfigOption(cli::CLIOptionBuilder()
                                          .setKey("n")
                                          .setDescription("name1 name2 ...")
                                          .setIsOptional(true)
                                          .setHasValue(true)
                                          .setHasMultipleValues(true)
                                          .build());

        _manager.registerConfigOption(
            cli::CLIOptionBuilder()
                .setKey("c")
                .setDescription("clientsNb")
                .setIsOptional(true)
                .setHasValue(true)
                .setHasMultipleValues(false)
                .setDefaultValue(std::to_string(server::defaultMaxTeamClient))
                .build());

        _manager.registerConfigOption(
            cli::CLIOptionBuilder()
                .setKey("f")
                .setDescription("freq")
                .setIsOptional(true)
                .setHasValue(true)
                .setHasMultipleValues(false)
                .setDefaultValue(std::to_string(server::defaultFreq))
                .build());
    }
} // namespace zappyServer
