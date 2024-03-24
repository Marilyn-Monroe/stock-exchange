#pragma once

#include <boost/container/vector.hpp>
#include <boost/unordered_map.hpp>

#include "level.hpp"
#include "order.hpp"
#include "order_book.hpp"
#include "symbol.hpp"
#include "user.hpp"

class MarketManager {
    friend class OrderBook;

public:
    typedef boost::container::vector<Symbol *> Symbols;
    typedef boost::container::vector<OrderBook *> OrderBooks;
    typedef boost::unordered_map<boost::uint64_t, OrderNode *> Orders;
    typedef boost::container::vector<User *> Users;

    MarketManager() : orders_count_(1) {

    }

    MarketManager(const MarketManager &) = delete;

    MarketManager(MarketManager &&) = delete;

    ~MarketManager();

    MarketManager &operator=(const MarketManager &) = delete;

    MarketManager &operator=(MarketManager &&) = delete;

    [[nodiscard]] const Symbols &symbols() const noexcept { return symbols_; }

    [[nodiscard]] const OrderBooks &order_books() const noexcept { return order_books_; }

    [[nodiscard]] const Orders &orders() const noexcept { return orders_; }

    [[nodiscard]] const Users &users() const noexcept { return users_; }

    [[nodiscard]] const Symbol *GetSymbol(boost::uint64_t id) const noexcept {
        return ((id < symbols_.size()) ? symbols_[id] : nullptr);
    }

    [[nodiscard]] const OrderBook *GetOrderBook(boost::uint64_t id) const noexcept {
        return ((id < order_books_.size()) ? order_books_[id] : nullptr);
    }

    [[nodiscard]] const Order *GetOrder(boost::uint64_t id) const noexcept {
        if (id == 0)
            return nullptr;

        auto it = orders_.find(id);
        return ((it != orders_.end()) ? it->second : nullptr);
    }

    [[nodiscard]] const User *GetUser(boost::uint64_t id) const noexcept {
        return ((id < users_.size()) ? users_[id] : nullptr);
    }

    [[nodiscard]] boost::uint64_t GetOrdersCount() const noexcept {
        return orders_count_;
    }

    ErrorCode AddSymbol(const Symbol &symbol);

    ErrorCode DeleteSymbol(boost::uint64_t id);

    ErrorCode AddOrderBook(const Symbol &symbol);

    ErrorCode DeleteOrderBook(boost::uint64_t id);

    ErrorCode AddOrder(const Order &order);

    ErrorCode DeleteOrder(boost::uint64_t id);

    ErrorCode AddUser(const User &user);

    ErrorCode DeleteUser(boost::uint64_t id);

private:
    Symbols symbols_;
    OrderBooks order_books_;
    Orders orders_;
    Users users_;

    boost::uint64_t orders_count_;

    ErrorCode ReduceOrder(boost::uint64_t id, boost::uint64_t quantity, bool recursive);

    ErrorCode DeleteOrder(boost::uint64_t id, bool recursive);

    void Match(OrderBook *order_book_ptr);

    void MatchLimit(OrderBook *order_book_ptr, Order *order_ptr);

    void MatchOrder(OrderBook *order_book_ptr, Order *order_ptr);

    bool ActivateStopOrders(OrderBook *order_book_ptr);

    bool ActivateStopOrders(OrderBook *order_book_ptr, LevelNode *level_ptr, boost::uint64_t stop_price);

    bool ActivateStopOrder(OrderBook *order_book_ptr, OrderNode *order_ptr);

    void RecalculateTrailingStopPrice(OrderBook *order_book_ptr, LevelNode *level_ptr);
};
