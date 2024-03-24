#include <gtest/gtest.h>

#include "../src/market_manager.hpp"

class MarketManagerOrderTest : public ::testing::Test {
protected:
    MarketManager market_manager;
    const Symbol test_symbol{0, "USDRUB"};
    const User test_user{0, "user"};

    void SetUp() override {
        market_manager.AddSymbol(test_symbol);
        market_manager.AddOrderBook(test_symbol);
        market_manager.AddUser(test_user);
    }

    void TearDown() override {
        market_manager.DeleteUser(test_user.Id);
        market_manager.DeleteOrderBook(test_symbol.Id);
        market_manager.DeleteSymbol(test_symbol.Id);
    }
};

TEST_F(MarketManagerOrderTest, AddOrderTest) {
    const Order order = Order::Buy(1, test_symbol.Id, test_user.Id, 62, 10);
    EXPECT_EQ(ErrorCode::OK, market_manager.AddOrder(order));

    const Symbol non_existent_symbol(1, "EURRUB");
    const Order invalid_order = Order::Buy(2, non_existent_symbol.Id, test_user.Id, 63, 20);
    EXPECT_EQ(ErrorCode::ORDER_BOOK_NOT_FOUND, market_manager.AddOrder(invalid_order));

    // TODO: need something to do with auto delete without manually call
    // The container only stores a reference and that you must make sure that the inserted elements stay alive longer than the container.
    market_manager.DeleteOrder(order.Id);
}

TEST_F(MarketManagerOrderTest, DeleteOrderTest) {
    const Order order = Order::Buy(1, test_symbol.Id, test_user.Id, 62, 10);
    market_manager.AddOrder(order);

    EXPECT_EQ(ErrorCode::OK, market_manager.DeleteOrder(order.Id));

    EXPECT_EQ(ErrorCode::ORDER_NOT_FOUND, market_manager.DeleteOrder(order.Id));

    // TODO: need something to do with auto delete without manually call
    // The container only stores a reference and that you must make sure that the inserted elements stay alive longer than the container.
    market_manager.DeleteOrder(order.Id);
}

TEST_F(MarketManagerOrderTest, GetOrderTest) {
    const Order order1 = Order::Buy(1, test_symbol.Id, test_user.Id, 62, 10);
    const Order order2 = Order::Buy(2, test_symbol.Id, test_user.Id, 63, 20);
    market_manager.AddOrder(order1);
    market_manager.AddOrder(order2);

    EXPECT_EQ(order1.Price, market_manager.GetOrder(order1.Id)->Price);
    EXPECT_EQ(order2.Price, market_manager.GetOrder(order2.Id)->Price);

    const Order non_existent_order = Order::Buy(3, test_symbol.Id, test_user.Id, 65, 10);
    EXPECT_EQ(nullptr, market_manager.GetOrder(non_existent_order.Id));

    // TODO: need something to do with auto delete without manually call
    // The container only stores a reference and that you must make sure that the inserted elements stay alive longer than the container.
    market_manager.DeleteOrder(order1.Id);
    market_manager.DeleteOrder(order2.Id);
}
