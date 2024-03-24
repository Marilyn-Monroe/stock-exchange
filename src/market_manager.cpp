#include "market_manager.hpp"

MarketManager::~MarketManager() {
    for (auto &order: orders_)
        delete order.second;
    orders_.clear();

    for (auto &order_book_ptr: order_books_)
        delete order_book_ptr;
    order_books_.clear();

    for (auto &symbol_ptr: symbols_)
        delete symbol_ptr;
    symbols_.clear();
}

ErrorCode MarketManager::AddSymbol(const Symbol &symbol) {
    if (symbols_.size() <= symbol.Id)
        symbols_.resize(symbol.Id + 1, nullptr);

    auto *symbol_ptr = new Symbol(symbol);

    if (symbols_[symbol.Id] != nullptr) {
        delete symbol_ptr;
        return ErrorCode::SYMBOL_DUPLICATE;
    }
    symbols_[symbol.Id] = symbol_ptr;

    return ErrorCode::OK;
}

ErrorCode MarketManager::DeleteSymbol(boost::uint64_t id) {
    if ((symbols_.size() <= id) || (symbols_[id] == nullptr))
        return ErrorCode::SYMBOL_NOT_FOUND;

    Symbol *symbol_ptr = symbols_[id];

    symbols_[id] = nullptr;

    delete symbol_ptr;

    return ErrorCode::OK;
}

ErrorCode MarketManager::AddOrderBook(const Symbol &symbol) {
    if ((symbols_.size() <= symbol.Id) || (symbols_[symbol.Id] == nullptr))
        return ErrorCode::SYMBOL_NOT_FOUND;

    Symbol *symbol_ptr = symbols_[symbol.Id];

    if (order_books_.size() <= symbol.Id)
        order_books_.resize(symbol.Id + 1, nullptr);

    auto *order_book_ptr = new OrderBook(*symbol_ptr);

    if (order_books_[symbol.Id] != nullptr) {
        delete order_book_ptr;
        return ErrorCode::ORDER_BOOK_DUPLICATE;
    }
    order_books_[symbol.Id] = order_book_ptr;

    return ErrorCode::OK;
}

ErrorCode MarketManager::DeleteOrderBook(boost::uint64_t id) {
    if ((order_books_.size() <= id) || (order_books_[id] == nullptr))
        return ErrorCode::ORDER_BOOK_NOT_FOUND;

    OrderBook *order_book_ptr = order_books_[id];

    order_books_[id] = nullptr;

    delete order_book_ptr;

    return ErrorCode::OK;
}

ErrorCode MarketManager::AddOrder(const Order &order) {
    auto *order_book_ptr = (OrderBook *) GetOrderBook(order.SymbolId);
    if (order_book_ptr == nullptr)
        return ErrorCode::ORDER_BOOK_NOT_FOUND;

    Order new_order(order);

    orders_count_++;

    MatchLimit(order_book_ptr, &new_order);

    if ((new_order.LeavesQuantity > 0)) {
        auto *order_ptr = new OrderNode(new_order);

        if (!orders_.insert(std::make_pair(order_ptr->Id, order_ptr)).second) {
            delete order_ptr;

            return ErrorCode::ORDER_DUPLICATE;
        }

        order_book_ptr->AddOrder(order_ptr);
    }

    Match(order_book_ptr);

    order_book_ptr->ResetMatchingPrice();

    return ErrorCode::OK;
}

ErrorCode MarketManager::ReduceOrder(boost::uint64_t id, boost::uint64_t quantity, bool recursive) {
    if (id == 0)
        return ErrorCode::ORDER_ID_INVALID;

    if (quantity == 0)
        return ErrorCode::ORDER_QUANTITY_INVALID;

    auto order_it = orders_.find(id);

    if (order_it == orders_.end())
        return ErrorCode::ORDER_NOT_FOUND;
    auto *order_ptr = (OrderNode *) order_it->second;

    auto *order_book_ptr = (OrderBook *) GetOrderBook(order_ptr->SymbolId);
    if (order_book_ptr == nullptr)
        return ErrorCode::ORDER_BOOK_NOT_FOUND;

    quantity = std::min(quantity, order_ptr->LeavesQuantity);

    boost::uint64_t hidden = order_ptr->HiddenQuantity();
    boost::uint64_t visible = order_ptr->VisibleQuantity();

    order_ptr->LeavesQuantity -= quantity;

    hidden -= order_ptr->HiddenQuantity();
    visible -= order_ptr->VisibleQuantity();

    if (order_ptr->LeavesQuantity > 0) {
        order_book_ptr->ReduceOrder(order_ptr, quantity, hidden, visible);
    } else {
        order_book_ptr->ReduceOrder(order_ptr, quantity, hidden, visible);

        orders_.erase(order_it);

        delete order_ptr;
    }

    if (!recursive)
        Match(order_book_ptr);

    order_book_ptr->ResetMatchingPrice();

    return ErrorCode::OK;
}

ErrorCode MarketManager::DeleteOrder(boost::uint64_t id) {
    return DeleteOrder(id, false);
}

ErrorCode MarketManager::DeleteOrder(boost::uint64_t id, bool recursive) {
    if (id == 0)
        return ErrorCode::ORDER_ID_INVALID;

    auto order_it = orders_.find(id);

    if (order_it == orders_.end())
        return ErrorCode::ORDER_NOT_FOUND;
    auto *order_ptr = (OrderNode *) order_it->second;

    auto *order_book_ptr = (OrderBook *) GetOrderBook(order_ptr->SymbolId);
    if (order_book_ptr == nullptr)
        return ErrorCode::ORDER_BOOK_NOT_FOUND;

    order_book_ptr->DeleteOrder(order_ptr);

    orders_.erase(order_it);

    delete order_ptr;

    if (!recursive)
        Match(order_book_ptr);

    order_book_ptr->ResetMatchingPrice();

    return ErrorCode::OK;
}

ErrorCode MarketManager::AddUser(const User &user) {
    if (users_.size() <= user.Id)
        users_.resize(user.Id + 1, nullptr);

    auto *user_ptr = new User(user);

    if (users_[user.Id] != nullptr) {
        delete user_ptr;
        return ErrorCode::USER_DUPLICATE;
    }
    users_[user.Id] = user_ptr;

    return ErrorCode::OK;
}

ErrorCode MarketManager::DeleteUser(boost::uint64_t id) {
    if ((users_.size() <= id) || (users_[id] == nullptr))
        return ErrorCode::USER_NOT_FOUND;

    User *user_ptr = users_[id];

    users_[id] = nullptr;

    delete user_ptr;

    return ErrorCode::OK;
}

void MarketManager::Match(OrderBook *order_book_ptr) {
    for (;;) {
        while ((order_book_ptr->best_bid_ != nullptr) &&
               (order_book_ptr->best_ask_ != nullptr) &&
               (order_book_ptr->best_bid_->Price >= order_book_ptr->best_ask_->Price)) {
            LevelNode *bid_level_ptr = order_book_ptr->best_bid_;
            LevelNode *ask_level_ptr = order_book_ptr->best_ask_;

            OrderNode *bid_order_ptr = &bid_level_ptr->OrderList.front();
            OrderNode *ask_order_ptr = &ask_level_ptr->OrderList.front();

            while ((bid_order_ptr != nullptr) && (ask_order_ptr != nullptr)) {
                auto *next_bid_order_ptr = static_cast<OrderNode *>(bid_order_ptr->next_);
                auto *next_ask_order_ptr = static_cast<OrderNode *>(ask_order_ptr->next_);

                OrderNode *executing_order_ptr = bid_order_ptr;
                OrderNode *reducing_order_ptr = ask_order_ptr;
                if (executing_order_ptr->LeavesQuantity > reducing_order_ptr->LeavesQuantity)
                    std::swap(executing_order_ptr, reducing_order_ptr);

                boost::uint64_t quantity = executing_order_ptr->LeavesQuantity;

                boost::uint64_t price = executing_order_ptr->Price;

                order_book_ptr->UpdateLastPrice(*executing_order_ptr, price);
                order_book_ptr->UpdateMatchingPrice(*executing_order_ptr, price);

                executing_order_ptr->ExecutedQuantity += quantity;
                if (executing_order_ptr->Side == OrderSide::BUY)
                    users_[executing_order_ptr->UserId]->Balance -= quantity * price;
                else
                    users_[executing_order_ptr->UserId]->Balance += quantity * price;

                DeleteOrder(executing_order_ptr->Id, true);

                order_book_ptr->UpdateLastPrice(*reducing_order_ptr, price);
                order_book_ptr->UpdateMatchingPrice(*reducing_order_ptr, price);

                reducing_order_ptr->ExecutedQuantity += quantity;
                if (reducing_order_ptr->Side == OrderSide::BUY)
                    users_[reducing_order_ptr->UserId]->Balance -= quantity * price;
                else
                    users_[reducing_order_ptr->UserId]->Balance += quantity * price;

                ReduceOrder(reducing_order_ptr->Id, quantity, true);

                bid_order_ptr = next_bid_order_ptr;
                ask_order_ptr = next_ask_order_ptr;
            }

            ActivateStopOrders(order_book_ptr, (LevelNode *) order_book_ptr->best_buy_stop(),
                               order_book_ptr->GetMarketPriceAsk());
            ActivateStopOrders(order_book_ptr, (LevelNode *) order_book_ptr->best_sell_stop(),
                               order_book_ptr->GetMarketPriceBid());
        }

        if (!ActivateStopOrders(order_book_ptr))
            break;
    }
}

void MarketManager::MatchLimit(OrderBook *order_book_ptr, Order *order_ptr) {
    MatchOrder(order_book_ptr, order_ptr);
}

void MarketManager::MatchOrder(OrderBook *order_book_ptr, Order *order_ptr) {
    LevelNode *level_ptr;
    while ((level_ptr = order_ptr->IsBuy() ? order_book_ptr->best_ask_ : order_book_ptr->best_bid_) != nullptr) {
        bool arbitrage = order_ptr->IsBuy() ? (order_ptr->Price >= level_ptr->Price) : (order_ptr->Price <=
                                                                                        level_ptr->Price);
        if (!arbitrage)
            return;

        OrderNode *executing_order_ptr = &level_ptr->OrderList.front();

        while (executing_order_ptr != nullptr) {
            auto *next_executing_order_ptr = static_cast<OrderNode *>(executing_order_ptr->next_);

            boost::uint64_t quantity = std::min(executing_order_ptr->LeavesQuantity, order_ptr->LeavesQuantity);

            boost::uint64_t price = executing_order_ptr->Price;

            order_book_ptr->UpdateLastPrice(*executing_order_ptr, price);
            order_book_ptr->UpdateMatchingPrice(*executing_order_ptr, price);

            executing_order_ptr->ExecutedQuantity += quantity;
            if (executing_order_ptr->Side == OrderSide::BUY)
                users_[executing_order_ptr->UserId]->Balance -= quantity * price;
            else
                users_[executing_order_ptr->UserId]->Balance += quantity * price;

            ReduceOrder(executing_order_ptr->Id, quantity, true);

            order_book_ptr->UpdateLastPrice(*order_ptr, price);
            order_book_ptr->UpdateMatchingPrice(*order_ptr, price);

            order_ptr->ExecutedQuantity += quantity;
            if (order_ptr->Side == OrderSide::BUY)
                users_[order_ptr->UserId]->Balance -= quantity * price;
            else
                users_[order_ptr->UserId]->Balance += quantity * price;

            order_ptr->LeavesQuantity -= quantity;
            if (order_ptr->LeavesQuantity == 0)
                return;

            executing_order_ptr = next_executing_order_ptr;
        }
    }
}

bool MarketManager::ActivateStopOrders(OrderBook *order_book_ptr) {
    bool result = false;
    bool stop = false;

    while (!stop) {
        stop = true;

        if (ActivateStopOrders(order_book_ptr, (LevelNode *) order_book_ptr->best_buy_stop(),
                               order_book_ptr->GetMarketPriceAsk()) ||
            ActivateStopOrders(order_book_ptr, (LevelNode *) order_book_ptr->best_trailing_buy_stop(),
                               order_book_ptr->GetMarketPriceAsk())) {
            result = true;
            stop = false;
        }

        RecalculateTrailingStopPrice(order_book_ptr, order_book_ptr->best_ask_);

        if (ActivateStopOrders(order_book_ptr, (LevelNode *) order_book_ptr->best_sell_stop(),
                               order_book_ptr->GetMarketPriceBid()) ||
            ActivateStopOrders(order_book_ptr, (LevelNode *) order_book_ptr->best_trailing_sell_stop(),
                               order_book_ptr->GetMarketPriceBid())) {
            result = true;
            stop = false;
        }

        RecalculateTrailingStopPrice(order_book_ptr, order_book_ptr->best_bid_);
    }

    return result;
}

bool MarketManager::ActivateStopOrders(OrderBook *order_book_ptr, LevelNode *level_ptr, boost::uint64_t stop_price) {
    bool result = false;

    if (level_ptr != nullptr) {
        bool arbitrage = level_ptr->IsBid() ? (stop_price <= level_ptr->Price) : (stop_price >= level_ptr->Price);
        if (!arbitrage)
            return result;

        OrderNode *activating_order_ptr = &level_ptr->OrderList.front();

        while (activating_order_ptr != nullptr) {
            auto *next_activating_order_ptr = static_cast<OrderNode *>(activating_order_ptr->next_);

            result = ActivateStopOrder(order_book_ptr, activating_order_ptr);

            activating_order_ptr = next_activating_order_ptr;
        }
    }

    return result;
}

bool MarketManager::ActivateStopOrder(OrderBook *order_book_ptr, OrderNode *order_ptr) {
    order_book_ptr->DeleteStopOrder(order_ptr);

    order_ptr->StopPrice = 0;

    MatchLimit(order_book_ptr, order_ptr);

    if ((order_ptr->LeavesQuantity > 0)) {
        order_book_ptr->AddOrder(order_ptr);
    } else {
        orders_.erase(orders_.find(order_ptr->Id));

        delete order_ptr;
    }

    return true;
}

void MarketManager::RecalculateTrailingStopPrice(OrderBook *order_book_ptr, LevelNode *level_ptr) {
    if (level_ptr == nullptr)
        return;

    boost::uint64_t new_trailing_price;

    if (level_ptr->Type == LevelType::ASK) {
        boost::uint64_t old_trailing_price = order_book_ptr->trailing_ask_price_;
        new_trailing_price = order_book_ptr->GetMarketTrailingStopPriceAsk();
        order_book_ptr->trailing_ask_price_ = new_trailing_price;
        if (new_trailing_price >= old_trailing_price)
            return;
    }
    if (level_ptr->Type == LevelType::BID) {
        boost::uint64_t old_trailing_price = order_book_ptr->trailing_bid_price_;
        new_trailing_price = order_book_ptr->GetMarketTrailingStopPriceBid();
        order_book_ptr->trailing_bid_price_ = new_trailing_price;
        if (new_trailing_price <= old_trailing_price)
            return;
    }

    LevelNode *previous = nullptr;
    LevelNode *current = (level_ptr->Type == LevelType::ASK) ? order_book_ptr->best_trailing_buy_stop_
                                                             : order_book_ptr->best_trailing_sell_stop_;
    while (current != nullptr) {
        bool recalculated = false;

        OrderNode *order_ptr = &current->OrderList.front();

        while (order_ptr != nullptr) {
            auto *next_order_ptr = static_cast<OrderNode *>(order_ptr->next_);

            boost::uint64_t old_stop_price = order_ptr->StopPrice;
            boost::uint64_t new_stop_price = order_book_ptr->CalculateTrailingStopPrice(*order_ptr);

            if (new_stop_price != old_stop_price) {
                order_book_ptr->DeleteTrailingStopOrder(order_ptr);

                order_book_ptr->AddTrailingStopOrder(order_ptr);

                recalculated = true;
            }

            order_ptr = next_order_ptr;
        }

        if (recalculated) {
            current = (previous != nullptr) ? previous : ((level_ptr->Type == LevelType::ASK)
                                                          ? order_book_ptr->best_trailing_buy_stop_
                                                          : order_book_ptr->best_trailing_sell_stop_);
        } else {
            previous = current;
            current = order_book_ptr->GetNextTrailingStopLevel(current);
        }
    }
}
