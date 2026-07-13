/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** main
*/

#include <iostream>
#include <ostream>

#include "network_server/Server.hpp"
#include "ZappyServerCore.hpp"

int main(int ac, char **av)
{
    try {
        zappyServer::ZappyServerCore core(ac, av);
        server::Server server(core.getServerConfig());
        server.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return server::endpointFailure;
    }
    return 0;
}
