#pragma once

#include <algorithm>
#include <limits>

#include <boost/cstdint.hpp>
#include <boost/intrusive/list.hpp>

#include "errors.hpp"

enum class OrderSide : boost::uint8_t {
    BUY,
    SELL
};

class Order {
public:
    boost::uint64_t Id;
    boost::uint64_t SymbolId;
    boost::uint64_t UserId;
    OrderSide Side;
    boost::uint64_t Price;
    boost::uint64_t StopPrice;

    boost::uint64_t Quantity;
    boost::uint64_t ExecutedQuantity;
    boost::uint64_t LeavesQuantity;

    boost::uint64_t MaxVisibleQuantity;

    [[nodiscard]] boost::uint64_t HiddenQuantity() const noexcept {
        return (LeavesQuantity > MaxVisibleQuantity) ? (LeavesQuantity - MaxVisibleQuantity) : 0;
    }

    [[nodiscard]] boost::uint64_t VisibleQuantity() const noexcept {
        return std::min(LeavesQuantity, MaxVisibleQuantity);
    }

    boost::int64_t TrailingDistance;
    boost::int64_t TrailingStep;

    Order() noexcept = default;

    Order(boost::uint64_t id, boost::uint64_t symbol, boost::uint64_t user, OrderSide side, boost::uint64_t price,
          boost::uint64_t stop_price, boost::uint64_t quantity,
          boost::uint64_t max_visible_quantity = std::numeric_limits<boost::uint64_t>::max(),
          boost::int64_t trailing_distance = 0,
          boost::int64_t trailing_step = 0) noexcept: Id(id),
                                                      SymbolId(symbol),
                                                      UserId(user),
                                                      Side(side),
                                                      Price(price),
                                                      StopPrice(stop_price),
                                                      Quantity(quantity),
                                                      ExecutedQuantity(0),
                                                      LeavesQuantity(quantity),
                                                      MaxVisibleQuantity(max_visible_quantity),
                                                      TrailingDistance(trailing_distance),
                                                      TrailingStep(trailing_step) {
    }

    Order(const Order &) noexcept = default;

    Order(Order &&) noexcept = default;

    ~Order() noexcept = default;

    Order &operator=(const Order &) noexcept = default;

    Order &operator=(Order &&) noexcept = default;

    [[nodiscard]] bool IsBuy() const noexcept { return Side == OrderSide::BUY; }

    [[nodiscard]] bool IsSell() const noexcept { return Side == OrderSide::SELL; }

    static Order Buy(boost::uint64_t id, boost::uint64_t symbol, boost::uint64_t user, boost::uint64_t price,
                     boost::uint64_t quantity,
                     boost::uint64_t max_visible_quantity = std::numeric_limits<boost::uint64_t>::max()) noexcept {
        return {id, symbol, user, OrderSide::BUY, price, 0, quantity, max_visible_quantity, 0, 0};
    }

    static Order Sell(boost::uint64_t id, boost::uint64_t symbol, boost::uint64_t user, boost::uint64_t price,
                      boost::uint64_t quantity,
                      boost::uint64_t max_visible_quantity = std::numeric_limits<boost::uint64_t>::max()) noexcept {
        return {id, symbol, user, OrderSide::SELL, price, 0, quantity, max_visible_quantity, 0, 0};
    }
};

class LevelNode;

class OrderNode : public Order, public boost::intrusive::list_base_hook<> {
public:
    LevelNode *Level;
    boost::intrusive::list_member_hook<> member_hook_;

    OrderNode(const Order &order) noexcept: Order(order), Level(nullptr) {
    }

    OrderNode(const OrderNode &) noexcept = default;

    OrderNode(OrderNode &&) noexcept = default;

    ~OrderNode() noexcept = default;

    OrderNode &operator=(const Order &order) noexcept {
        Order::operator=(order);
        Level = nullptr;
        return *this;
    }

    OrderNode &operator=(const OrderNode &) noexcept = default;

    OrderNode &operator=(OrderNode &&) noexcept = default;
};

typedef boost::intrusive::list<OrderNode, boost::intrusive::member_hook<OrderNode, boost::intrusive::list_member_hook<>, &OrderNode::member_hook_>> OrderNodeList;
