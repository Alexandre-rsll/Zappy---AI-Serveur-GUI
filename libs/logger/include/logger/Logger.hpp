/*
** EPITECH PROJECT, 2026
** liblogger
** File description:
** Logger
*/

#ifndef LIBLOGGER_LOGGER_HPP
#define LIBLOGGER_LOGGER_HPP

#include <iostream>
#include <ostream>
#include <source_location>
#include <streambuf>
#include <string>
#include <utility>

#include "logger/Level.hpp"

namespace logger {

    class NullBuffer : public std::streambuf {
    protected:
        int overflow(const int chr) override
        {
            return chr;
        }
    };

    class Logger {
    public:
        explicit Logger(
            std::ostream &stream, std::string context,
            const LevelPower minLevel = Level::defaultLevel.getLevelPower())
            : _minPower(minLevel), _stream(stream),
              _context(std::move(context)), _nullStream(&_nullBuffer)
        {}

        explicit Logger(
            std::ostream &stream = std::cout,
            const LevelPower minLevel = Level::defaultLevel.getLevelPower(),
            const std::source_location location =
                std::source_location::current())
            : _minPower(minLevel), _stream(stream),
              _context(extractContext(location.function_name())),
              _nullStream(&_nullBuffer)
        {}

        std::ostream &log(const Level &level,
                          const std::source_location &location =
                              std::source_location::current());

        std::ostream &debug(const std::source_location &location =
                                std::source_location::current());

        std::ostream &info(const std::source_location &location =
                               std::source_location::current());

        std::ostream &warning(const std::source_location &location =
                                  std::source_location::current());

        std::ostream &error(const std::source_location &location =
                                std::source_location::current());

        void setLevel(LevelPower level);

    private:
        static std::string extractContext(const std::string &functionName);

        void writePrefix(const Level &level,
                         const std::source_location &location) const;

        LevelPower _minPower;
        std::ostream &_stream;
        std::string _context;

        NullBuffer _nullBuffer;
        std::ostream _nullStream;
    };
} // namespace logger

#endif // LIBLOGGER_LOGGER_HPP
