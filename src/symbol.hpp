#pragma once

#include <string>

#include <boost/cstdint.hpp>

class Symbol {
public:
    boost::uint64_t Id;
    std::string Name;

    Symbol() noexcept = default;

    Symbol(boost::uint64_t id, std::string name) noexcept: Id(id), Name(std::move(name)) {

    }

    Symbol(const Symbol &) noexcept = default;

    Symbol(Symbol &&) noexcept = default;

    ~Symbol() noexcept = default;

    Symbol &operator=(const Symbol &) noexcept = default;

    Symbol &operator=(Symbol &&) noexcept = default;
};
