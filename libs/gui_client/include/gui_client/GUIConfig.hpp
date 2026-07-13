/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** GUIConfig file
*/

#ifndef GUICONFIG_HPP
#define GUICONFIG_HPP

#include <string>

namespace gui {

    struct GUIConfig {
        int port = 0;
        std::string machine = "localhost";
    };

} // namespace gui

#endif /* !GUICONFIG_HPP */
