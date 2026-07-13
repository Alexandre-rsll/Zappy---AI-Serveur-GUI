/*
** EPITECH PROJECT, 2026
** liblogger
** File description:
** Level
*/

#include "logger/Level.hpp"

namespace logger {

    const Level Level::debug = Level("DEBUG", 0, Color(0x1CE31C));
    const Level Level::info = Level("INFO", 10, Color(0x9C9C9C));
    const Level Level::warning = Level("WARN", 20, Color(0xF56A00));
    const Level Level::error = Level("ERROR", 30, Color(0xD10000));

    const Level &Level::defaultLevel =
#ifdef DEBUG
        Level::debug;
#else  // DEBUG
        Level::info;
#endif // DEBUG

} // namespace logger
