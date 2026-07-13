/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Protocol
*/

#include "protocol/Protocol.hpp"

namespace protocol {
    std::string Protocol::_quantities(const Quantities &q)
    {
        std::string out;
        for (const std::size_t value : q) {
            out += " " + std::to_string(value);
        }
        return out;
    }

    std::pair<std::string, std::string>
    Protocol::splitVerbArg(const std::string &line)
    {
        const auto pos = line.find(' ');
        if (pos == std::string::npos) {
            return {line, ""};
        }
        return {line.substr(0, pos), line.substr(pos + 1)};
    }

    std::string Protocol::msz(const int x, const int y)
    {
        return "msz " + std::to_string(x) + " " + std::to_string(y) + "\n";
    }

    std::string Protocol::bct(const int x, const int y, const Quantities &q)
    {
        return "bct " + std::to_string(x) + " " + std::to_string(y) +
               _quantities(q) + "\n";
    }

    std::string Protocol::tna(const std::string &name)
    {
        return "tna " + name + "\n";
    }

    std::string Protocol::sgt(const int t)
    {
        return "sgt " + std::to_string(t) + "\n";
    }

    std::string Protocol::sst(const int t)
    {
        return "sst " + std::to_string(t) + "\n";
    }

    std::string Protocol::pnw(const NewPlayer &player)
    {
        return "pnw #" + std::to_string(player.n) + " " +
               std::to_string(player.x) + " " + std::to_string(player.y) + " " +
               std::to_string(player.o) + " " + std::to_string(player.l) + " " +
               player.team + "\n";
    }

    std::string Protocol::ppo(const int n, const int x, const int y,
                              const int o)
    {
        return "ppo #" + std::to_string(n) + " " + std::to_string(x) + " " +
               std::to_string(y) + " " + std::to_string(o) + "\n";
    }

    std::string Protocol::plv(const int n, const int l)
    {
        return "plv #" + std::to_string(n) + " " + std::to_string(l) + "\n";
    }

    std::string Protocol::pin(const int n, const int x, const int y,
                              const Quantities &q)
    {
        return "pin #" + std::to_string(n) + " " + std::to_string(x) + " " +
               std::to_string(y) + _quantities(q) + "\n";
    }

    std::string Protocol::pex(const int n)
    {
        return "pex #" + std::to_string(n) + "\n";
    }

    std::string Protocol::pbc(const int n, const std::string &msg)
    {
        return "pbc #" + std::to_string(n) + " " + msg + "\n";
    }

    std::string Protocol::pic(const int x, const int y, const int l,
                              const std::vector<int> &ids)
    {
        std::string out = "pic " + std::to_string(x) + " " + std::to_string(y) +
                          " " + std::to_string(l);
        for (const int id : ids) {
            out += " #" + std::to_string(id);
        }
        return out + "\n";
    }

    std::string Protocol::pie(const int x, const int y, const int r)
    {
        return "pie " + std::to_string(x) + " " + std::to_string(y) + " " +
               std::to_string(r) + "\n";
    }

    std::string Protocol::pfk(const int n)
    {
        return "pfk #" + std::to_string(n) + "\n";
    }

    std::string Protocol::pdr(const int n, const int i)
    {
        return "pdr #" + std::to_string(n) + " " + std::to_string(i) + "\n";
    }

    std::string Protocol::pgt(const int n, const int i)
    {
        return "pgt #" + std::to_string(n) + " " + std::to_string(i) + "\n";
    }

    std::string Protocol::pdi(const int n)
    {
        return "pdi #" + std::to_string(n) + "\n";
    }

    std::string Protocol::enw(const int e, const int n, const int x,
                              const int y)
    {
        return "enw #" + std::to_string(e) + " #" + std::to_string(n) + " " +
               std::to_string(x) + " " + std::to_string(y) + "\n";
    }

    std::string Protocol::ebo(const int e)
    {
        return "ebo #" + std::to_string(e) + "\n";
    }

    std::string Protocol::edi(const int e)
    {
        return "edi #" + std::to_string(e) + "\n";
    }

    std::string Protocol::seg(const std::string &team)
    {
        return "seg " + team + "\n";
    }

    std::string Protocol::smg(const std::string &msg)
    {
        return "smg " + msg + "\n";
    }

    std::string Protocol::suc()
    {
        return "suc\n";
    }

    std::string Protocol::sbp()
    {
        return "sbp\n";
    }
} // namespace protocol
