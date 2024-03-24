#include <gtest/gtest.h>

#include "../src/market_manager.hpp"

class MarketManagerOrderBookTest : public ::testing::Test {
protected:
    MarketManager market_manager;
    const Symbol test_symbol{0, "USDRUB"};

    void SetUp() override {

    }

    void TearDown() override {

    }
};

TEST_F(MarketManagerOrderBookTest, AddOrderBookTest) {
    market_manager.AddSymbol(test_symbol);

    EXPECT_EQ(ErrorCode::OK, market_manager.AddOrderBook(test_symbol));

    const Symbol non_existent_symbol(1, "EURRUB");
    EXPECT_EQ(ErrorCode::SYMBOL_NOT_FOUND, market_manager.AddOrderBook(non_existent_symbol));

    EXPECT_EQ(ErrorCode::ORDER_BOOK_DUPLICATE, market_manager.AddOrderBook(test_symbol));
}

TEST_F(MarketManagerOrderBookTest, DeleteOrderBookTest) {
    market_manager.AddSymbol(test_symbol);
    market_manager.AddOrderBook(test_symbol);

    EXPECT_EQ(ErrorCode::OK, market_manager.DeleteOrderBook(test_symbol.Id));

    EXPECT_EQ(ErrorCode::ORDER_BOOK_NOT_FOUND, market_manager.DeleteOrderBook(test_symbol.Id));
}

TEST_F(MarketManagerOrderBookTest, GetOrderBookTest) {
    market_manager.AddSymbol(test_symbol);
    market_manager.AddOrderBook(test_symbol);

    EXPECT_EQ(test_symbol.Id, market_manager.GetOrderBook(test_symbol.Id)->symbol().Id);

    const Symbol non_existent_symbol(1, "EURRUB");
    EXPECT_EQ(nullptr, market_manager.GetOrderBook(non_existent_symbol.Id));
}
