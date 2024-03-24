#pragma once

#include <string>

#include <boost/cstdint.hpp>

class User {
public:
    boost::uint64_t Id;
    std::string Name;
    boost::int64_t Balance;

    User(boost::uint64_t id) noexcept: Id(id), Name(), Balance(0) {

    }

    User(boost::uint64_t id, std::string name) noexcept: Id(id), Name(std::move(name)), Balance(0) {

    }

    User(const User &) noexcept = default;

    User(User &&) noexcept = default;

    ~User() noexcept = default;

    User &operator=(const User &) noexcept = default;

    User &operator=(User &&) noexcept = default;

    friend bool operator==(const User &user1, const User &user2) noexcept { return user1.Id == user2.Id; }

    friend bool operator!=(const User &user1, const User &user2) noexcept { return user1.Id != user2.Id; }

    friend bool operator<(const User &user1, const User &user2) noexcept { return user1.Id < user2.Id; }

    friend bool operator>(const User &user1, const User &user2) noexcept { return user1.Id > user2.Id; }

    friend bool operator<=(const User &user1, const User &user2) noexcept { return user1.Id <= user2.Id; }

    friend bool operator>=(const User &user1, const User &user2) noexcept { return user1.Id >= user2.Id; }
};
