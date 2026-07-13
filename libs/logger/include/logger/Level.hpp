/*
** EPITECH PROJECT, 2026
** raytracer
** File description:
** Level
*/

#ifndef LIBLOGGER_LEVEL_HPP
#define LIBLOGGER_LEVEL_HPP

#include <optional>
#include <string>
#include <utility>

#include "logger/Color.hpp"

namespace logger {
    using LevelPower = std::uint32_t;

    class Level {
    public:
        explicit Level(std::string name, const LevelPower levelPower)
            : _name(std::move(name)), _levelPower(levelPower),
              _color(std::nullopt)
        {}

        explicit Level(std::string name, const LevelPower levelPower,
                       const Color color)
            : _name(std::move(name)), _levelPower(levelPower), _color(color)
        {}

        [[nodiscard]] const std::string &getName() const
        {
            return _name;
        }
        [[nodiscard]] LevelPower getLevelPower() const
        {
            return _levelPower;
        }
        [[nodiscard]] const std::optional<Color> &getColor() const
        {
            return _color;
        }

        static const Level &defaultLevel;

        static const Level debug;
        static const Level info;
        static const Level warning;
        static const Level error;

    private:
        std::string _name;
        LevelPower _levelPower;
        std::optional<Color> _color;
    };
} // namespace logger

#endif // LIBLOGGER_LEVEL_HPP
