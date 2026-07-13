/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** ClientRunner
*/

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cli/CLIManager.hpp"
#include "gui_client/GUIConfig.hpp"
#include "gui_client/ClientRunner.hpp"
#include "gui_client/GUIClient.hpp"
#include "gui_client/rendering/WorldRenderer.hpp"

namespace gui {
    static void registerOptions(cli::CLIManager &manager)
    {
        manager.registerConfigOption(cli::CLIOptionBuilder()
                                         .setKey("p")
                                         .setDescription("port")
                                         .setIsOptional(false)
                                         .setHasValue(true)
                                         .build());

        manager.registerConfigOption(cli::CLIOptionBuilder()
                                         .setKey("h")
                                         .setDescription("machine")
                                         .setIsOptional(false)
                                         .setHasValue(true)
                                         .build());
    }

    static std::string getRequiredValue(const cli::CLIManager &manager,
                                        const std::string &key)
    {
        const auto &config = manager.getConfig(key);

        if (!config.value.has_value()) {
            throw std::runtime_error("Missing value for option: -" + key);
        }
        return *config.value;
    }

    static int getRequiredInt(const cli::CLIManager &manager,
                              const std::string &key)
    {
        return std::stoi(getRequiredValue(manager, key));
    }

    static GUIConfig buildConfig(const cli::CLIManager &manager)
    {
        GUIConfig config{};

        config.port = getRequiredInt(manager, "p");
        config.machine = getRequiredValue(manager, "h");
        if (config.port <= 0) {
            throw std::runtime_error("Port must be positive");
        }
        return config;
    }

    int ClientRunner::run(int ac, char **av)
    {
        try {
            cli::CLIManager manager;
            std::vector<std::string> params(av + 1, av + ac);

            registerOptions(manager);
            if (!manager.parse(params)) {
                return 0;
            }
            GUIConfig config = buildConfig(manager);
            GUIClient client(config);
            WorldRenderer renderer;

            client.startProtocolThread();
            while (client.isRunning() && !renderer.shouldClose()) {
                const GameState state = client.getGameStateSnapshot();

                renderer.render(state);
            }
            client.stop();
        } catch (const std::exception &exception) {
            std::cerr << exception.what() << std::endl;
            return gui::endpointFailure;
        }
        return 0;
    }
} // namespace gui
