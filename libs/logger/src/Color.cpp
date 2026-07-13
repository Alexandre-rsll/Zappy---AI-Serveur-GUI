/*
** EPITECH PROJECT, 2026
** liblogger
** File description:
** Color
*/

#include "logger/Color.hpp"

namespace logger {
    const Color Color::red = Color(0xD10000);
    const Color Color::green = Color(0x1CE31C);
    const Color Color::blue = Color(0x1C58E3);
    const Color Color::black = Color(0x000000);
    const Color Color::white = Color(0xFFFFFF);
    const Color Color::yellow = Color(0xFCFF2E);
    const Color Color::orange = Color(0xF56A00);
    const Color Color::cyan = Color(0x00FFFF);
    const Color Color::magenta = Color(0xFF00FF);
    const Color Color::grey = Color(0x9C9C9C);
    const Color Color::reset = Color();

    std::ostream &operator<<(std::ostream &out, const Color &color)
    {
        if (color.isReset()) {
            return out << "\033[0m";
        }
        return out << "\033[38;2;" << static_cast<int>(color.getR()) << ';'
                   << static_cast<int>(color.getG()) << ';'
                   << static_cast<int>(color.getB()) << "m";
    }
} // namespace logger
