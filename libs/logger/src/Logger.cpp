/*
** EPITECH PROJECT, 2026
** liblogger
** File description:
** Logger
*/

#include "logger/Logger.hpp"

namespace logger {
    static std::size_t findLastScopeAt(const std::string &name,
                                       const std::size_t end)
    {
        std::size_t lastScope = std::string::npos;
        int templateDepth = 0;

        for (std::size_t i = 0; i < end; ++i) {
            if (name[i] == '<') {
                ++templateDepth;
            } else if (name[i] == '>') {
                --templateDepth;
            } else if (i + 1 < end && templateDepth == 0 && name[i] == ':' &&
                       name[i + 1] == ':') {
                lastScope = i;
                ++i;
            }
        }
        return lastScope;
    }

    static void stripNamespacePrefixes(std::string &result)
    {
        std::size_t pos = 0;
        while ((pos = result.find("::", pos)) != std::string::npos) {
            std::size_t start = pos;
            while (start > 0) {
                const char chr = result[start - 1];
                if (std::isalnum(static_cast<unsigned char>(chr)) == 0 &&
                    chr != '_') {
                    break;
                }
                --start;
            }
            result.erase(start, pos + 2 - start);
            pos = start;
        }
    }

    std::string Logger::extractContext(const std::string &functionName)
    {
        const std::size_t parenthesis = functionName.find('(');
        const std::size_t end = parenthesis == std::string::npos
                                    ? functionName.size()
                                    : parenthesis;

        const std::size_t lastScope = findLastScopeAt(functionName, end);

        if (lastScope == std::string::npos) {
            std::string result = functionName.substr(0, end);
            stripNamespacePrefixes(result);
            return result;
        }

        const std::size_t previousScope =
            findLastScopeAt(functionName, lastScope);
        const std::size_t start =
            previousScope == std::string::npos ? 0 : previousScope + 2;
        std::string result = functionName.substr(start, lastScope - start);
        stripNamespacePrefixes(result);
        return result;
    }

    void Logger::writePrefix(const Level &level,
                             const std::source_location &location) const
    {
        const auto &colorOpt = level.getColor();
        if (colorOpt.has_value()) {
            _stream << *colorOpt;
        }

        _stream << '[' << level.getName() << ']';
        if (colorOpt.has_value()) {
            _stream << Color::reset;
        }

#ifdef DEBUG
        _stream << " [" << location.file_name() << ':' << location.line()
                << ']';
#endif // DEBUG
        _stream << " [" << _context << "] : ";
    }

    void Logger::setLevel(const LevelPower level)
    {
        _minPower = level;
    }

    std::ostream &Logger::error(const std::source_location &location)
    {
        return log(Level::error, location);
    }

    std::ostream &Logger::warning(const std::source_location &location)
    {
        return log(Level::warning, location);
    }

    std::ostream &Logger::info(const std::source_location &location)
    {
        return log(Level::info, location);
    }

    std::ostream &Logger::debug(const std::source_location &location)
    {
        return log(Level::debug, location);
    }

    std::ostream &Logger::log(const Level &level,
                              const std::source_location &location)
    {
        if (level.getLevelPower() < _minPower) {
            return _nullStream;
        }

        writePrefix(level, location);
        return _stream;
    }
} // namespace logger
