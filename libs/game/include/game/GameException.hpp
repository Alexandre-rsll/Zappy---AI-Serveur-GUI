/*
** EPITECH PROJECT, 2026
** zappy
** File description:
** GameException
*/

#ifndef ZAPPY_GAMEEXCEPTION_HPP
#define ZAPPY_GAMEEXCEPTION_HPP

#include <exception>
#include <iostream>
#include <sstream>

class GameException : public std::exception {
public:
    explicit GameException(const std::string &msg) : _message(msg)
    {}
    explicit GameException(const std::string &msg, const std::exception &reason)
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

#endif // ZAPPY_GAMEEXCEPTION_HPP
