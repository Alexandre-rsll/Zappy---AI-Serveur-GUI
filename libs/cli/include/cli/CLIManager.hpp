/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** CLIManager
*/

#ifndef CLIMANAGER_HPP
#define CLIMANAGER_HPP

#include <unordered_map>
#include <vector>
#include <sstream>

#include "cli/CLIOption.hpp"
#include "cli/CLIOptionConfig.hpp"
#include "logger/Logger.hpp"

namespace cli {

    class CLIManagerException : public std::exception {
    public:
        explicit CLIManagerException(const std::string &msg) : _message(msg)
        {}
        explicit CLIManagerException(const std::string &msg,
                                     const std::exception &reason)
            : _message(msg)
        {
            std::stringstream buf;

            buf << msg << std::endl << "Because:" << std::endl << reason.what();
            _message = buf.str();
        }

        [[nodiscard]] const char *what() const noexcept override
        {
            return _message.c_str();
        }

    private:
        std::string _message;
    };

    class CLIManager {
    public:
        CLIManager() = default;
        ~CLIManager() = default;

        void registerConfigOption(const CLIOption &configOption);
        [[nodiscard]]
        bool parse(const std::vector<std::string> &params);
        bool hasConfig(const std::string &key) const;
        const CLIOptionConfig &getConfig(const std::string &key) const;
        void printHelp(std::ostream &stream) const noexcept;

    private:
    private:
        void parseMultipleValues(
            const std::vector<std::string> &params, std::size_t &index,
            const CLIOption &option,
            const std::unordered_map<std::string, const CLIOption *>
                &optionByKey);
        void parseSingleValue(
            const std::vector<std::string> &params, std::size_t &index,
            const CLIOption &option,
            const std::unordered_map<std::string, const CLIOption *>
                &optionByKey);
        std::vector<CLIOption> _options;
        std::unordered_map<std::string, CLIOptionConfig> _results;
        logger::Logger _logger;
    };

} // namespace cli

#endif // CLIMANAGER_HPP
