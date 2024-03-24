#pragma once

#include <boost/intrusive/set.hpp>

#include "order.hpp"
#include "update.hpp"

enum class LevelType : boost::uint8_t {
    BID,
    ASK
};

class Level {
public:
    LevelType Type;
    boost::uint64_t Price;
    boost::uint64_t TotalVolume;
    boost::uint64_t HiddenVolume;
    boost::uint64_t VisibleVolume;
    size_t Orders;

    Level(LevelType type, boost::uint64_t price) noexcept: Type(type),
                                                           Price(price),
                                                           TotalVolume(0),
                                                           HiddenVolume(0),
                                                           VisibleVolume(0),
                                                           Orders(0) {
    }

    Level(const Level &) noexcept = default;

    Level(Level &&) noexcept = default;

    ~Level() noexcept = default;

    Level &operator=(const Level &) noexcept = default;

    Level &operator=(Level &&) noexcept = default;

    friend bool operator==(const Level &level1, const Level &level2) noexcept { return level1.Price == level2.Price; }

    friend bool operator!=(const Level &level1, const Level &level2) noexcept { return level1.Price != level2.Price; }

    friend bool operator<(const Level &level1, const Level &level2) noexcept { return level1.Price < level2.Price; }

    friend bool operator>(const Level &level1, const Level &level2) noexcept { return level1.Price > level2.Price; }

    friend bool operator<=(const Level &level1, const Level &level2) noexcept { return level1.Price <= level2.Price; }

    friend bool operator>=(const Level &level1, const Level &level2) noexcept { return level1.Price >= level2.Price; }

    [[nodiscard]] bool IsBid() const noexcept { return Type == LevelType::BID; }

    [[nodiscard]] bool IsAsk() const noexcept { return Type == LevelType::ASK; }
};

class LevelNode : public Level, public boost::intrusive::set_base_hook<> {
public:
    OrderNodeList OrderList;
    boost::intrusive::set_member_hook<> member_hook_;

    LevelNode(LevelType type, boost::uint64_t price) noexcept: Level(type, price) {
    }

    LevelNode(const Level &level) noexcept: Level(level) {
    }

    LevelNode(const LevelNode &) noexcept = default;

    LevelNode(LevelNode &&) noexcept = default;

    ~LevelNode() noexcept = default;

    LevelNode &operator=(const Level &level) noexcept {
        Level::operator=(level);
        OrderList.clear();
        return *this;
    }

    LevelNode &operator=(const LevelNode &) noexcept = default;

    LevelNode &operator=(LevelNode &&) noexcept = default;

    friend bool operator==(const LevelNode &level1, const LevelNode &level2) noexcept {
        return level1.Price == level2.Price;
    }

    friend bool operator!=(const LevelNode &level1, const LevelNode &level2) noexcept {
        return level1.Price != level2.Price;
    }

    friend bool operator<(const LevelNode &level1, const LevelNode &level2) noexcept {
        return level1.Price < level2.Price;
    }

    friend bool operator>(const LevelNode &level1, const LevelNode &level2) noexcept {
        return level1.Price > level2.Price;
    }

    friend bool operator<=(const LevelNode &level1, const LevelNode &level2) noexcept {
        return level1.Price <= level2.Price;
    }

    friend bool operator>=(const LevelNode &level1, const LevelNode &level2) noexcept {
        return level1.Price >= level2.Price;
    }
};

typedef boost::intrusive::set<LevelNode, boost::intrusive::member_hook<LevelNode, boost::intrusive::set_member_hook<>, &LevelNode::member_hook_>> LevelNodeSet;

class LevelUpdate {
public:
    UpdateType Type;
    Level Update;
    bool Top;

    LevelUpdate(UpdateType type, const Level &update, bool top) noexcept: Type(type),
                                                                          Update(update),
                                                                          Top(top) {
    }

    LevelUpdate(const LevelUpdate &) noexcept = default;

    LevelUpdate(LevelUpdate &&) noexcept = default;

    ~LevelUpdate() noexcept = default;

    LevelUpdate &operator=(const LevelUpdate &) noexcept = default;

    LevelUpdate &operator=(LevelUpdate &&) noexcept = default;
};
