#include <gtest/gtest.h>

#include "../src/market_manager.hpp"

class MarketManagerSymbolTest : public ::testing::Test {
protected:
    MarketManager market_manager;
    const Symbol test_symbol{0, "USDRUB"};

    void SetUp() override {

    }

    void TearDown() override {

    }
};

TEST_F(MarketManagerSymbolTest, AddSymbolTest) {
    EXPECT_EQ(ErrorCode::OK, market_manager.AddSymbol(test_symbol));

    EXPECT_EQ(ErrorCode::SYMBOL_DUPLICATE, market_manager.AddSymbol(test_symbol));
}

TEST_F(MarketManagerSymbolTest, DeleteSymbolTest) {
    market_manager.AddSymbol(test_symbol);

    EXPECT_EQ(ErrorCode::OK, market_manager.DeleteSymbol(test_symbol.Id));

    EXPECT_EQ(ErrorCode::SYMBOL_NOT_FOUND, market_manager.DeleteSymbol(test_symbol.Id));
}

TEST_F(MarketManagerSymbolTest, GetSymbolTest) {
    market_manager.AddSymbol(test_symbol);

    EXPECT_EQ(test_symbol.Id, market_manager.GetSymbol(test_symbol.Id)->Id);

    const Symbol non_existent_symbol(1, "EURRUB");
    EXPECT_EQ(nullptr, market_manager.GetSymbol(non_existent_symbol.Id));
}
