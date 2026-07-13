/*
** EPITECH PROJECT, 2026
** liblogger
** File description:
** Color
*/

#ifndef LIBLOGGER_COLOR_HPP
#define LIBLOGGER_COLOR_HPP

#include <cstdint>
#include <ostream>

namespace logger {
    class Color {
    public:
        static constexpr std::uint32_t redShift = 16U;
        static constexpr std::uint32_t greenShift = 8U;
        static constexpr std::uint32_t channelMask = 0xFFU;

        explicit Color(const std::uint8_t red, const std::uint8_t green,
                       const std::uint8_t blue)
            : _r(red), _g(green), _b(blue), _isReset(false)
        {}

        explicit Color() : _r(0), _g(0), _b(0), _isReset(true)
        {}

        explicit Color(const std::uint32_t hex)
            : _r(static_cast<std::uint8_t>((hex >> redShift) & channelMask)),
              _g(static_cast<std::uint8_t>((hex >> greenShift) & channelMask)),
              _b(static_cast<std::uint8_t>(hex & channelMask)), _isReset(false)
        {}

        [[nodiscard]] std::uint8_t getR() const
        {
            return _r;
        }
        [[nodiscard]] std::uint8_t getG() const
        {
            return _g;
        }
        [[nodiscard]] std::uint8_t getB() const
        {
            return _b;
        }
        [[nodiscard]] bool isReset() const
        {
            return _isReset;
        }

        void setR(const std::uint8_t red)
        {
            _r = red;
        }
        void setG(const std::uint8_t green)
        {
            _g = green;
        }
        void setB(const std::uint8_t blue)
        {
            _b = blue;
        }
        void setIsReset(const bool isReset)
        {
            _isReset = isReset;
        }

        static const Color red;
        static const Color green;
        static const Color blue;
        static const Color black;
        static const Color white;
        static const Color yellow;
        static const Color orange;
        static const Color cyan;
        static const Color magenta;
        static const Color grey;
        static const Color reset;

    private:
        std::uint8_t _r;
        std::uint8_t _g;
        std::uint8_t _b;
        bool _isReset;
    };

    std::ostream &operator<<(std::ostream &out, const Color &color);
} // namespace logger

#endif // LIBLOGGER_COLOR_HPP
