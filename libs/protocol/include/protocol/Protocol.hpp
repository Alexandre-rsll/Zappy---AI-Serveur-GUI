/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Protocol
*/

#ifndef ZAPPY_PROTOCOL_HPP
#define ZAPPY_PROTOCOL_HPP

#include <array>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace protocol {
    class Protocol {
    public:
        static constexpr std::size_t resourceCount = 7;
        using Quantities = std::array<std::size_t, resourceCount>;

        struct NewPlayer {
            int n;
            int x;
            int y;
            int o;
            int l;
            std::string team;
        };

        static std::pair<std::string, std::string>
        splitVerbArg(const std::string &line);

        static std::string msz(int x, int y);
        static std::string bct(int x, int y, const Quantities &q);
        static std::string tna(const std::string &name);
        static std::string sgt(int t);
        static std::string sst(int t);
        static std::string pnw(const NewPlayer &player);
        static std::string ppo(int n, int x, int y, int o);
        static std::string plv(int n, int l);
        static std::string pin(int n, int x, int y, const Quantities &q);
        static std::string pex(int n);
        static std::string pbc(int n, const std::string &msg);
        static std::string pic(int x, int y, int l,
                               const std::vector<int> &ids);
        static std::string pie(int x, int y, int r);
        static std::string pfk(int n);
        static std::string pdr(int n, int i);
        static std::string pgt(int n, int i);
        static std::string pdi(int n);
        static std::string enw(int e, int n, int x, int y);
        static std::string ebo(int e);
        static std::string edi(int e);
        static std::string seg(const std::string &team);
        static std::string smg(const std::string &msg);
        static std::string suc();
        static std::string sbp();

    private:
        static std::string _quantities(const Quantities &q);
    };
} // namespace protocol

#endif // ZAPPY_PROTOCOL_HPP
