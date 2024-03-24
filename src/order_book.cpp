#include "order_book.hpp"

OrderBook::OrderBook(Symbol symbol)
        : symbol_(std::move(symbol)),
          best_bid_(nullptr),
          best_ask_(nullptr),
          best_buy_stop_(nullptr),
          best_sell_stop_(nullptr),
          best_trailing_buy_stop_(nullptr),
          best_trailing_sell_stop_(nullptr),
          last_bid_price_(0),
          last_ask_price_(std::numeric_limits<boost::uint64_t>::max()),
          matching_bid_price_(0),
          matching_ask_price_(std::numeric_limits<boost::uint64_t>::max()),
          trailing_bid_price_(0),
          trailing_ask_price_(std::numeric_limits<boost::uint64_t>::max()) {
}

OrderBook::~OrderBook() {
    for (auto &bid: bids_)
        delete &bid;
    bids_.clear();

    for (auto &ask: asks_)
        delete &ask;
    asks_.clear();

    for (auto &buy_stop: buy_stop_)
        delete &buy_stop;
    buy_stop_.clear();

    for (auto &sell_stop: sell_stop_)
        delete &sell_stop;
    sell_stop_.clear();

    for (auto &trailing_buy_stop: trailing_buy_stop_)
        delete &trailing_buy_stop;
    trailing_buy_stop_.clear();

    for (auto &trailing_sell_stop: trailing_sell_stop_)
        delete &trailing_sell_stop;
    trailing_sell_stop_.clear();
}

LevelNode *OrderBook::AddLevel(OrderNode *order_ptr) {
    LevelNode *level_ptr;

    if (order_ptr->IsBuy()) {
        level_ptr = new LevelNode(LevelType::BID, order_ptr->Price);

        bids_.insert(*level_ptr);

        if ((best_bid_ == nullptr) || (level_ptr->Price > best_bid_->Price))
            best_bid_ = level_ptr;
    } else {
        level_ptr = new LevelNode(LevelType::ASK, order_ptr->Price);

        asks_.insert(*level_ptr);

        if ((best_ask_ == nullptr) || (level_ptr->Price < best_ask_->Price))
            best_ask_ = level_ptr;
    }

    return level_ptr;
}

LevelNode *OrderBook::DeleteLevel(OrderNode *order_ptr) {
    LevelNode *level_ptr = order_ptr->Level;

    if (order_ptr->IsBuy()) {
        if (level_ptr == best_bid_)
            best_bid_ = GetNextLevel(best_bid_);

        bids_.erase(LevelNodeSet::iterator(LevelNodeSet::s_iterator_to(*level_ptr)));
    } else {
        if (level_ptr == best_ask_)
            best_ask_ = GetNextLevel(best_ask_);

        asks_.erase(LevelNodeSet::iterator(LevelNodeSet::s_iterator_to(*level_ptr)));
    }

    delete level_ptr;

    return nullptr;
}

LevelUpdate OrderBook::AddOrder(OrderNode *order_ptr) {
    LevelNode *level_ptr = order_ptr->IsBuy() ? (LevelNode *) GetBid(order_ptr->Price) : (LevelNode *) GetAsk(
            order_ptr->Price);

    UpdateType update = UpdateType::UPDATE;
    if (level_ptr == nullptr) {
        level_ptr = AddLevel(order_ptr);
        update = UpdateType::ADD;
    }

    level_ptr->TotalVolume += order_ptr->LeavesQuantity;
    level_ptr->HiddenVolume += order_ptr->HiddenQuantity();
    level_ptr->VisibleVolume += order_ptr->VisibleQuantity();

    level_ptr->OrderList.push_back(*order_ptr);
    ++level_ptr->Orders;

    order_ptr->Level = level_ptr;

    return {update, *order_ptr->Level, (order_ptr->Level == (order_ptr->IsBuy() ? best_bid_ : best_ask_))};
}

LevelUpdate OrderBook::ReduceOrder(OrderNode *order_ptr, boost::uint64_t quantity, boost::uint64_t hidden,
                                   boost::uint64_t visible) {
    LevelNode *level_ptr = order_ptr->Level;

    level_ptr->TotalVolume -= quantity;
    level_ptr->HiddenVolume -= hidden;
    level_ptr->VisibleVolume -= visible;

    if (order_ptr->LeavesQuantity == 0) {
        level_ptr->OrderList.erase(level_ptr->OrderList.iterator_to(*order_ptr));
        --level_ptr->Orders;
    }

    Level level(*level_ptr);

    UpdateType update = UpdateType::UPDATE;
    if (level_ptr->TotalVolume == 0) {
        order_ptr->Level = DeleteLevel(order_ptr);
        update = UpdateType::DELETE;
    }

    return {update, level, ((order_ptr->Level == nullptr) ||
                            (order_ptr->Level == (order_ptr->IsBuy() ? best_bid_ : best_ask_)))};
}

LevelUpdate OrderBook::DeleteOrder(OrderNode *order_ptr) {
    LevelNode *level_ptr = order_ptr->Level;

    // Update the price level volume
    level_ptr->TotalVolume -= order_ptr->LeavesQuantity;
    level_ptr->HiddenVolume -= order_ptr->HiddenQuantity();
    level_ptr->VisibleVolume -= order_ptr->VisibleQuantity();

    level_ptr->OrderList.erase(level_ptr->OrderList.iterator_to(*order_ptr));
    --level_ptr->Orders;

    Level level(*level_ptr);

    UpdateType update = UpdateType::UPDATE;
    if (level_ptr->TotalVolume == 0) {
        order_ptr->Level = DeleteLevel(order_ptr);
        update = UpdateType::DELETE;
    }

    return {update, level, ((order_ptr->Level == nullptr) ||
                            (order_ptr->Level == (order_ptr->IsBuy() ? best_bid_ : best_ask_)))};
}

LevelNode *OrderBook::AddStopLevel(OrderNode *order_ptr) {
    LevelNode *level_ptr;

    if (order_ptr->IsBuy()) {
        level_ptr = new LevelNode(LevelType::ASK, order_ptr->StopPrice);

        buy_stop_.insert(*level_ptr);

        if ((best_buy_stop_ == nullptr) || (level_ptr->Price < best_buy_stop_->Price))
            best_buy_stop_ = level_ptr;
    } else {
        level_ptr = new LevelNode(LevelType::BID, order_ptr->StopPrice);

        sell_stop_.insert(*level_ptr);

        if ((best_sell_stop_ == nullptr) || (level_ptr->Price > best_sell_stop_->Price))
            best_sell_stop_ = level_ptr;
    }

    return level_ptr;
}

LevelNode *OrderBook::DeleteStopLevel(OrderNode *order_ptr) {
    LevelNode *level_ptr = order_ptr->Level;

    if (order_ptr->IsBuy()) {
        if (level_ptr == best_buy_stop_)
            best_buy_stop_ = GetNextStopLevel(best_buy_stop_);

        buy_stop_.erase(LevelNodeSet::iterator(LevelNodeSet::s_iterator_to(*level_ptr)));
    } else {
        if (level_ptr == best_sell_stop_)
            best_sell_stop_ = GetNextStopLevel(best_sell_stop_);

        sell_stop_.erase(LevelNodeSet::iterator(LevelNodeSet::s_iterator_to(*level_ptr)));
    }

    delete level_ptr;

    return nullptr;
}

void OrderBook::DeleteStopOrder(OrderNode *order_ptr) {
    LevelNode *level_ptr = order_ptr->Level;

    level_ptr->TotalVolume -= order_ptr->LeavesQuantity;
    level_ptr->HiddenVolume -= order_ptr->HiddenQuantity();
    level_ptr->VisibleVolume -= order_ptr->VisibleQuantity();

    level_ptr->OrderList.erase(level_ptr->OrderList.iterator_to(*order_ptr));
    --level_ptr->Orders;

    if (level_ptr->TotalVolume == 0) {
        order_ptr->Level = DeleteStopLevel(order_ptr);
    }
}

LevelNode *OrderBook::AddTrailingStopLevel(OrderNode *order_ptr) {
    LevelNode *level_ptr;

    if (order_ptr->IsBuy()) {
        level_ptr = new LevelNode(LevelType::ASK, order_ptr->StopPrice);

        trailing_buy_stop_.insert(*level_ptr);

        if ((best_trailing_buy_stop_ == nullptr) || (level_ptr->Price < best_trailing_buy_stop_->Price))
            best_trailing_buy_stop_ = level_ptr;
    } else {
        level_ptr = new LevelNode(LevelType::BID, order_ptr->StopPrice);

        trailing_sell_stop_.insert(*level_ptr);

        if ((best_trailing_sell_stop_ == nullptr) || (level_ptr->Price > best_trailing_sell_stop_->Price))
            best_trailing_sell_stop_ = level_ptr;
    }

    return level_ptr;
}

LevelNode *OrderBook::DeleteTrailingStopLevel(OrderNode *order_ptr) {
    LevelNode *level_ptr = order_ptr->Level;

    if (order_ptr->IsBuy()) {
        if (level_ptr == best_trailing_buy_stop_)
            best_trailing_buy_stop_ = GetNextTrailingStopLevel(best_trailing_buy_stop_);

        trailing_buy_stop_.erase(LevelNodeSet::iterator(LevelNodeSet::s_iterator_to(*level_ptr)));
    } else {
        if (level_ptr == best_trailing_sell_stop_)
            best_trailing_sell_stop_ = GetNextTrailingStopLevel(best_trailing_sell_stop_);

        trailing_sell_stop_.erase(LevelNodeSet::iterator(LevelNodeSet::s_iterator_to(*level_ptr)));
    }

    delete level_ptr;

    return nullptr;
}

void OrderBook::AddTrailingStopOrder(OrderNode *order_ptr) {
    LevelNode *level_ptr = order_ptr->IsBuy() ? (LevelNode *) GetTrailingBuyStopLevel(order_ptr->StopPrice)
                                              : (LevelNode *) GetTrailingSellStopLevel(order_ptr->StopPrice);

    if (level_ptr == nullptr)
        level_ptr = AddTrailingStopLevel(order_ptr);

    level_ptr->TotalVolume += order_ptr->LeavesQuantity;
    level_ptr->HiddenVolume += order_ptr->HiddenQuantity();
    level_ptr->VisibleVolume += order_ptr->VisibleQuantity();

    level_ptr->OrderList.push_back(*order_ptr);
    ++level_ptr->Orders;

    order_ptr->Level = level_ptr;
}

void OrderBook::DeleteTrailingStopOrder(OrderNode *order_ptr) {
    LevelNode *level_ptr = order_ptr->Level;

    level_ptr->TotalVolume -= order_ptr->LeavesQuantity;
    level_ptr->HiddenVolume -= order_ptr->HiddenQuantity();
    level_ptr->VisibleVolume -= order_ptr->VisibleQuantity();

    level_ptr->OrderList.erase(level_ptr->OrderList.iterator_to(*order_ptr));
    --level_ptr->Orders;

    if (level_ptr->TotalVolume == 0) {
        order_ptr->Level = DeleteTrailingStopLevel(order_ptr);
    }
}

boost::uint64_t OrderBook::CalculateTrailingStopPrice(const Order &order) const noexcept {
    boost::uint64_t market_price = order.IsBuy() ? GetMarketTrailingStopPriceAsk() : GetMarketTrailingStopPriceBid();
    boost::int64_t trailing_distance = order.TrailingDistance;
    boost::int64_t trailing_step = order.TrailingStep;

    if (trailing_distance < 0) {
        trailing_distance = (boost::int64_t) ((-trailing_distance * market_price) / 10000);
        trailing_step = (boost::int64_t) ((-trailing_step * market_price) / 10000);
    }

    boost::uint64_t old_price = order.StopPrice;

    if (order.IsBuy()) {
        boost::uint64_t new_price = (market_price < (std::numeric_limits<boost::uint64_t>::max() - trailing_distance))
                                    ? (market_price + trailing_distance) : std::numeric_limits<boost::uint64_t>::max();

        if (new_price < old_price)
            if ((old_price - new_price) >= (boost::uint64_t) trailing_step)
                return new_price;
    } else {
        boost::uint64_t new_price = (market_price > (boost::uint64_t) trailing_distance) ? (market_price -
                                                                                            trailing_distance) : 0;

        if (new_price > old_price)
            if ((new_price - old_price) >= (boost::uint64_t) trailing_step)
                return new_price;
    }

    return old_price;
}
