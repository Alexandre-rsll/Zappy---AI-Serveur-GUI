/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** ClientRunner file
*/

#ifndef ClientRunner_HPP
#define ClientRunner_HPP

namespace gui {

    constexpr int endpointFailure = 84;
    constexpr int guiLoopSleepMs = 100;
    class ClientRunner {
    public:
        static int run(int ac, char **av);
    };

} // namespace gui

#endif /* ClientRunner_HPP */
