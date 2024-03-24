#pragma once

#include "level.hpp"
#include "symbol.hpp"

class MarketManager;

class OrderBook {
    friend class MarketManager;

public:
    OrderBook(Symbol symbol);

    OrderBook(const OrderBook &) = delete;

    OrderBook(OrderBook &&) = delete;

    ~OrderBook();

    OrderBook &operator=(const OrderBook &) = delete;

    OrderBook &operator=(OrderBook &&) = delete;

    explicit operator bool() const noexcept { return !empty(); }

    [[nodiscard]] bool empty() const noexcept { return size() == 0; }

    [[nodiscard]] size_t size() const noexcept {
        return bids_.size() + asks_.size() + buy_stop_.size() + sell_stop_.size() + trailing_buy_stop_.size() +
               trailing_sell_stop_.size();
    }

    [[nodiscard]] const Symbol &symbol() const noexcept { return symbol_; }

    [[nodiscard]] const LevelNode *best_bid() const noexcept { return best_bid_; }

    [[nodiscard]] const LevelNode *best_ask() const noexcept { return best_ask_; }

    [[nodiscard]] const LevelNodeSet &bids() const noexcept { return bids_; }

    [[nodiscard]] const LevelNodeSet &asks() const noexcept { return asks_; }

    [[nodiscard]] const LevelNode *best_buy_stop() const noexcept { return best_buy_stop_; }

    [[nodiscard]] const LevelNode *best_sell_stop() const noexcept { return best_sell_stop_; }

    [[nodiscard]] const LevelNodeSet &buy_stop() const noexcept { return buy_stop_; }

    [[nodiscard]] const LevelNodeSet &sell_stop() const noexcept { return sell_stop_; }

    [[nodiscard]] const LevelNode *best_trailing_buy_stop() const noexcept { return best_trailing_buy_stop_; }

    [[nodiscard]] const LevelNode *best_trailing_sell_stop() const noexcept { return best_trailing_sell_stop_; }

    [[nodiscard]] const LevelNodeSet &trailing_buy_stop() const noexcept { return trailing_buy_stop_; }

    [[nodiscard]] const LevelNodeSet &trailing_sell_stop() const noexcept { return trailing_sell_stop_; }

    [[nodiscard]] const LevelNode *GetBid(boost::uint64_t price) const noexcept {
        auto it = bids_.find(LevelNode(LevelType::BID, price));
        return (it != bids_.end()) ? it.operator->() : nullptr;
    }

    [[nodiscard]] const LevelNode *GetAsk(boost::uint64_t price) const noexcept {
        auto it = asks_.find(LevelNode(LevelType::ASK, price));
        return (it != asks_.end()) ? it.operator->() : nullptr;
    }

    [[nodiscard]] const LevelNode *GetBuyStopLevel(boost::uint64_t price) const noexcept {
        auto it = buy_stop_.find(LevelNode(LevelType::ASK, price));
        return (it != buy_stop_.end()) ? it.operator->() : nullptr;
    }

    [[nodiscard]] const LevelNode *GetSellStopLevel(boost::uint64_t price) const noexcept {
        auto it = sell_stop_.find(LevelNode(LevelType::BID, price));
        return (it != sell_stop_.end()) ? it.operator->() : nullptr;
    }

    [[nodiscard]] const LevelNode *GetTrailingBuyStopLevel(boost::uint64_t price) const noexcept {
        auto it = trailing_buy_stop_.find(LevelNode(LevelType::ASK, price));
        return (it != trailing_buy_stop_.end()) ? it.operator->() : nullptr;
    }

    [[nodiscard]] const LevelNode *GetTrailingSellStopLevel(boost::uint64_t price) const noexcept {
        auto it = trailing_sell_stop_.find(LevelNode(LevelType::BID, price));
        return (it != trailing_sell_stop_.end()) ? it.operator->() : nullptr;
    }

private:
    Symbol symbol_;

    LevelNode *best_bid_;
    LevelNode *best_ask_;
    LevelNodeSet bids_;
    LevelNodeSet asks_;

    LevelNode *GetNextLevel(LevelNode *level) noexcept {
        if (level->IsBid()) {
            LevelNodeSet::reverse_iterator it(LevelNodeSet::s_iterator_to(*level));
            if (it == bids_.rend())
                return nullptr;
            ++it;
            return it.operator->();
        } else {
            LevelNodeSet::iterator it(LevelNodeSet::s_iterator_to(*level));
            ++it;
            if (it == asks_.end())
                return nullptr;
            return it.operator->();
        }
    }

    LevelNode *AddLevel(OrderNode *order_ptr);

    LevelNode *DeleteLevel(OrderNode *order_ptr);

    LevelUpdate AddOrder(OrderNode *order_ptr);

    LevelUpdate
    ReduceOrder(OrderNode *order_ptr, boost::uint64_t quantity, boost::uint64_t hidden, boost::uint64_t visible);

    LevelUpdate DeleteOrder(OrderNode *order_ptr);

    LevelNode *best_buy_stop_;
    LevelNode *best_sell_stop_;
    LevelNodeSet buy_stop_;
    LevelNodeSet sell_stop_;

    LevelNode *GetNextStopLevel(LevelNode *level) noexcept {
        if (level->IsBid()) {
            LevelNodeSet::reverse_iterator it(LevelNodeSet::s_iterator_to(*level));
            if (it == buy_stop_.rend())
                return nullptr;
            ++it;
            return it.operator->();
        } else {
            LevelNodeSet::iterator it(LevelNodeSet::s_iterator_to(*level));
            ++it;
            if (it == sell_stop_.end())
                return nullptr;
            return it.operator->();
        }
    }

    LevelNode *AddStopLevel(OrderNode *order_ptr);

    LevelNode *DeleteStopLevel(OrderNode *order_ptr);

    void DeleteStopOrder(OrderNode *order_ptr);

    LevelNode *best_trailing_buy_stop_;
    LevelNode *best_trailing_sell_stop_;
    LevelNodeSet trailing_buy_stop_;
    LevelNodeSet trailing_sell_stop_;

    LevelNode *GetNextTrailingStopLevel(LevelNode *level) noexcept {
        if (level->IsBid()) {
            LevelNodeSet::reverse_iterator it(LevelNodeSet::s_iterator_to(*level));
            if (it == trailing_buy_stop_.rend())
                return nullptr;
            ++it;
            return it.operator->();
        } else {
            LevelNodeSet::iterator it(LevelNodeSet::s_iterator_to(*level));
            ++it;
            if (it == trailing_sell_stop_.end())
                return nullptr;
            return it.operator->();
        }
    }

    LevelNode *AddTrailingStopLevel(OrderNode *order_ptr);

    LevelNode *DeleteTrailingStopLevel(OrderNode *order_ptr);

    void AddTrailingStopOrder(OrderNode *order_ptr);

    void DeleteTrailingStopOrder(OrderNode *order_ptr);

    [[nodiscard]] boost::uint64_t CalculateTrailingStopPrice(const Order &order) const noexcept;

    boost::uint64_t last_bid_price_;
    boost::uint64_t last_ask_price_;
    boost::uint64_t matching_bid_price_;
    boost::uint64_t matching_ask_price_;
    boost::uint64_t trailing_bid_price_;
    boost::uint64_t trailing_ask_price_;

    [[nodiscard]] boost::uint64_t GetMarketPriceBid() const noexcept {
        boost::uint64_t matching_price = matching_bid_price_;
        boost::uint64_t best_price = (best_bid_ != nullptr) ? best_bid_->Price : 0;
        return std::max(matching_price, best_price);
    }

    [[nodiscard]] boost::uint64_t GetMarketPriceAsk() const noexcept {
        boost::uint64_t matching_price = matching_ask_price_;
        boost::uint64_t best_price = (best_ask_ != nullptr) ? best_ask_->Price
                                                            : std::numeric_limits<boost::uint64_t>::max();
        return std::min(matching_price, best_price);
    }

    [[nodiscard]] boost::uint64_t GetMarketTrailingStopPriceBid() const noexcept {
        boost::uint64_t last_price = last_bid_price_;
        boost::uint64_t best_price = (best_bid_ != nullptr) ? best_bid_->Price : 0;
        return std::min(last_price, best_price);
    }

    [[nodiscard]] boost::uint64_t GetMarketTrailingStopPriceAsk() const noexcept {
        boost::uint64_t last_price = last_ask_price_;
        boost::uint64_t best_price = (best_ask_ != nullptr) ? best_ask_->Price
                                                            : std::numeric_limits<boost::uint64_t>::max();
        return std::max(last_price, best_price);
    }

    void UpdateLastPrice(const Order &order, boost::uint64_t price) noexcept {
        if (order.IsBuy())
            last_bid_price_ = price;
        else
            last_ask_price_ = price;
    }

    void UpdateMatchingPrice(const Order &order, boost::uint64_t price) noexcept {
        if (order.IsBuy())
            matching_bid_price_ = price;
        else
            matching_ask_price_ = price;
    }

    void ResetMatchingPrice() noexcept {
        matching_bid_price_ = 0;
        matching_ask_price_ = std::numeric_limits<boost::uint64_t>::max();
    }
};
