#include <gtest/gtest.h>

#include "../src/market_manager.hpp"

class MarketManagerTradingTest : public ::testing::Test {
protected:
    MarketManager market_manager;
    const Symbol test_symbol{0, "USDRUB"};
    const User test_user0{0, "user0"};
    const User test_user1{1, "user1"};
    const User test_user2{2, "user2"};

    void SetUp() override {
        market_manager.AddSymbol(test_symbol);
        market_manager.AddOrderBook(test_symbol);
        market_manager.AddUser(test_user0);
        market_manager.AddUser(test_user1);
        market_manager.AddUser(test_user2);
    }

    void TearDown() override {
        market_manager.DeleteUser(test_user0.Id);
        market_manager.DeleteUser(test_user1.Id);
        market_manager.DeleteUser(test_user2.Id);
        market_manager.DeleteOrderBook(test_symbol.Id);
        market_manager.DeleteSymbol(test_symbol.Id);
    }
};

TEST_F(MarketManagerTradingTest, MultipleOrdersMatchingBuy) {
    const Order order1 = Order::Buy(1, test_symbol.Id, test_user0.Id, 62, 10);
    market_manager.AddOrder(order1);

    EXPECT_EQ(1, market_manager.GetOrderBook(test_symbol.Id)->size());
    EXPECT_EQ(order1.Quantity, market_manager.GetOrderBook(test_symbol.Id)->GetBid(order1.Price)->TotalVolume);
    EXPECT_EQ(order1.Price, market_manager.GetOrderBook(test_symbol.Id)->best_bid()->Price);
    EXPECT_EQ(0, market_manager.GetUser(test_user0.Id)->Balance);

    const Order order2 = Order::Buy(2, test_symbol.Id, test_user1.Id, 63, 20);
    market_manager.AddOrder(order2);

    EXPECT_EQ(2, market_manager.GetOrderBook(test_symbol.Id)->size());
    EXPECT_EQ(order2.Quantity, market_manager.GetOrderBook(test_symbol.Id)->GetBid(order2.Price)->TotalVolume);
    EXPECT_EQ(order2.Price, market_manager.GetOrderBook(test_symbol.Id)->best_bid()->Price);
    EXPECT_EQ(0, market_manager.GetUser(test_user0.Id)->Balance);
    EXPECT_EQ(0, market_manager.GetUser(test_user1.Id)->Balance);

    const Order order3 = Order::Sell(3, test_symbol.Id, test_user2.Id, 61, 50);
    market_manager.AddOrder(order3);

    EXPECT_EQ(1, market_manager.GetOrderBook(test_symbol.Id)->size());
    EXPECT_EQ(order3.Quantity - (order1.Quantity + order2.Quantity),
              market_manager.GetOrderBook(test_symbol.Id)->GetAsk(order3.Price)->TotalVolume);
    EXPECT_EQ(order3.Price, market_manager.GetOrderBook(test_symbol.Id)->best_ask()->Price);
    EXPECT_EQ(-(order1.Price * order1.Quantity), market_manager.GetUser(test_user0.Id)->Balance);
    EXPECT_EQ(-(order2.Price * order2.Quantity), market_manager.GetUser(test_user1.Id)->Balance);
    EXPECT_EQ(((order2.Quantity * order2.Price) + (order1.Quantity * order1.Price)),
              market_manager.GetUser(test_user2.Id)->Balance);

    // TODO: need something to do with auto delete without manually call
    // The container only stores a reference and that you must make sure that the inserted elements stay alive longer than the container.
    market_manager.DeleteOrder(order3.Id);
}

TEST_F(MarketManagerTradingTest, PartiallyFilledOrderMatchingBuy) {
    const Order order1 = Order::Buy(1, test_symbol.Id, test_user0.Id, 200, 100);
    market_manager.AddOrder(order1);

    EXPECT_EQ(1, market_manager.GetOrderBook(test_symbol.Id)->size());
    EXPECT_EQ(order1.Quantity, market_manager.GetOrderBook(test_symbol.Id)->best_bid()->TotalVolume);
    EXPECT_EQ(order1.Price, market_manager.GetOrderBook(test_symbol.Id)->best_bid()->Price);
    EXPECT_EQ(0, market_manager.GetUser(test_user0.Id)->Balance);

    const Order order2 = Order::Sell(2, test_symbol.Id, test_user1.Id, 190, 50);
    market_manager.AddOrder(order2);

    EXPECT_EQ(1, market_manager.GetOrderBook(test_symbol.Id)->size());
    EXPECT_EQ(order1.Quantity - order2.Quantity, market_manager.GetOrderBook(test_symbol.Id)->best_bid()->TotalVolume);
    EXPECT_EQ(order1.Price, market_manager.GetOrderBook(test_symbol.Id)->best_bid()->Price);
    EXPECT_EQ(-(200 * 50), market_manager.GetUser(test_user0.Id)->Balance);
    EXPECT_EQ(200 * 50, market_manager.GetUser(test_user1.Id)->Balance);

    // TODO: need something to do with auto delete without manually call
    // The container only stores a reference and that you must make sure that the inserted elements stay alive longer than the container.
    market_manager.DeleteOrder(order1.Id);
}

TEST_F(MarketManagerTradingTest, FullyFilledOrderMatchingBuy) {
    const Order order1 = Order::Buy(1, test_symbol.Id, test_user0.Id, 200, 100);
    market_manager.AddOrder(order1);

    EXPECT_EQ(1, market_manager.GetOrderBook(test_symbol.Id)->size());
    EXPECT_EQ(order1.Quantity, market_manager.GetOrderBook(test_symbol.Id)->best_bid()->TotalVolume);
    EXPECT_EQ(order1.Price, market_manager.GetOrderBook(test_symbol.Id)->best_bid()->Price);
    EXPECT_EQ(0, market_manager.GetUser(test_user0.Id)->Balance);

    const Order order2 = Order::Sell(2, test_symbol.Id, test_user1.Id, 200, 100);
    market_manager.AddOrder(order2);

    EXPECT_EQ(0, market_manager.GetOrderBook(test_symbol.Id)->size());
    EXPECT_EQ(nullptr, market_manager.GetOrderBook(test_symbol.Id)->best_bid());
    EXPECT_EQ(nullptr, market_manager.GetOrderBook(test_symbol.Id)->best_ask());
    EXPECT_EQ(-(200 * 100), market_manager.GetUser(test_user0.Id)->Balance);
    EXPECT_EQ(200 * 100, market_manager.GetUser(test_user1.Id)->Balance);
}

TEST_F(MarketManagerTradingTest, UnfilledOrderMatchingBuy) {
    const Order order = Order::Buy(1, test_symbol.Id, test_user0.Id, 200, 100);
    market_manager.AddOrder(order);

    EXPECT_EQ(1, market_manager.GetOrderBook(test_symbol.Id)->size());
    EXPECT_EQ(order.Quantity, market_manager.GetOrderBook(test_symbol.Id)->best_bid()->TotalVolume);
    EXPECT_EQ(order.Price, market_manager.GetOrderBook(test_symbol.Id)->best_bid()->Price);
    EXPECT_EQ(0, market_manager.GetUser(test_user0.Id)->Balance);

    // TODO: need something to do with auto delete without manually call
    // The container only stores a reference and that you must make sure that the inserted elements stay alive longer than the container.
    market_manager.DeleteOrder(order.Id);
}

TEST_F(MarketManagerTradingTest, PartialFillWithCancelledOrderBuy) {
    const Order order1 = Order::Buy(1, test_symbol.Id, test_user0.Id, 200, 100);
    market_manager.AddOrder(order1);

    const Order order2 = Order::Sell(2, test_symbol.Id, test_user1.Id, 190, 50);
    market_manager.AddOrder(order2);

    market_manager.DeleteOrder(order1.Id);

    EXPECT_EQ(0, market_manager.GetOrderBook(test_symbol.Id)->size());
    EXPECT_EQ(nullptr, market_manager.GetOrderBook(test_symbol.Id)->best_bid());
    EXPECT_EQ(nullptr, market_manager.GetOrderBook(test_symbol.Id)->best_ask());
    EXPECT_EQ(-(200 * 50), market_manager.GetUser(test_user0.Id)->Balance);
    EXPECT_EQ(200 * 50, market_manager.GetUser(test_user1.Id)->Balance);

    // TODO: need something to do with auto delete without manually call
    // The container only stores a reference and that you must make sure that the inserted elements stay alive longer than the container.
    market_manager.DeleteOrder(order1.Id);
}

TEST_F(MarketManagerTradingTest, MultipleOrdersMatchingSell) {
    const Order order1 = Order::Sell(1, test_symbol.Id, test_user0.Id, 62, 10);
    market_manager.AddOrder(order1);

    EXPECT_EQ(1, market_manager.GetOrderBook(test_symbol.Id)->size());
    EXPECT_EQ(order1.Quantity, market_manager.GetOrderBook(test_symbol.Id)->GetAsk(order1.Price)->TotalVolume);
    EXPECT_EQ(order1.Price, market_manager.GetOrderBook(test_symbol.Id)->best_ask()->Price);
    EXPECT_EQ(0, market_manager.GetUser(test_user0.Id)->Balance);

    const Order order2 = Order::Sell(2, test_symbol.Id, test_user1.Id, 63, 20);
    market_manager.AddOrder(order2);

    EXPECT_EQ(2, market_manager.GetOrderBook(test_symbol.Id)->size());
    EXPECT_EQ(order2.Quantity, market_manager.GetOrderBook(test_symbol.Id)->GetAsk(order2.Price)->TotalVolume);
    EXPECT_EQ(order1.Price, market_manager.GetOrderBook(test_symbol.Id)->best_ask()->Price);
    EXPECT_EQ(0, market_manager.GetUser(test_user0.Id)->Balance);
    EXPECT_EQ(0, market_manager.GetUser(test_user1.Id)->Balance);

    const Order order3 = Order::Buy(3, test_symbol.Id, test_user2.Id, 65, 50);
    market_manager.AddOrder(order3);

    EXPECT_EQ(1, market_manager.GetOrderBook(test_symbol.Id)->size());
    EXPECT_EQ(order3.Quantity - (order1.Quantity + order2.Quantity),
              market_manager.GetOrderBook(test_symbol.Id)->GetBid(order3.Price)->TotalVolume);
    EXPECT_EQ(order3.Price, market_manager.GetOrderBook(test_symbol.Id)->best_bid()->Price);
    EXPECT_EQ(order1.Price * order1.Quantity, market_manager.GetUser(test_user0.Id)->Balance);
    EXPECT_EQ(order2.Price * order2.Quantity, market_manager.GetUser(test_user1.Id)->Balance);
    EXPECT_EQ(-((order2.Quantity * order2.Price) + (order1.Quantity * order1.Price)),
              market_manager.GetUser(test_user2.Id)->Balance);

    // TODO: need something to do with auto delete without manually call
    // The container only stores a reference and that you must make sure that the inserted elements stay alive longer than the container.
    market_manager.DeleteOrder(order3.Id);
}

TEST_F(MarketManagerTradingTest, PartiallyFilledOrderMatchingSell) {
    const Order order1 = Order::Sell(1, test_symbol.Id, test_user0.Id, 200, 100);
    market_manager.AddOrder(order1);

    EXPECT_EQ(1, market_manager.GetOrderBook(test_symbol.Id)->size());
    EXPECT_EQ(order1.Quantity, market_manager.GetOrderBook(test_symbol.Id)->best_ask()->TotalVolume);
    EXPECT_EQ(order1.Price, market_manager.GetOrderBook(test_symbol.Id)->best_ask()->Price);
    EXPECT_EQ(0, market_manager.GetUser(test_user0.Id)->Balance);

    const Order order2 = Order::Buy(2, test_symbol.Id, test_user1.Id, 210, 50);
    market_manager.AddOrder(order2);

    EXPECT_EQ(1, market_manager.GetOrderBook(test_symbol.Id)->size());
    EXPECT_EQ(order1.Quantity - order2.Quantity, market_manager.GetOrderBook(test_symbol.Id)->best_ask()->TotalVolume);
    EXPECT_EQ(order1.Price, market_manager.GetOrderBook(test_symbol.Id)->best_ask()->Price);
    EXPECT_EQ(200 * 50, market_manager.GetUser(test_user0.Id)->Balance);
    EXPECT_EQ(-(200 * 50), market_manager.GetUser(test_user1.Id)->Balance);

    // TODO: need something to do with auto delete without manually call
    // The container only stores a reference and that you must make sure that the inserted elements stay alive longer than the container.
    market_manager.DeleteOrder(order1.Id);
}

TEST_F(MarketManagerTradingTest, FullyFilledOrderMatchingSell) {
    const Order order1 = Order::Sell(1, test_symbol.Id, test_user0.Id, 200, 100);
    market_manager.AddOrder(order1);

    EXPECT_EQ(1, market_manager.GetOrderBook(test_symbol.Id)->size());
    EXPECT_EQ(order1.Quantity, market_manager.GetOrderBook(test_symbol.Id)->best_ask()->TotalVolume);
    EXPECT_EQ(order1.Price, market_manager.GetOrderBook(test_symbol.Id)->best_ask()->Price);
    EXPECT_EQ(0, market_manager.GetUser(test_user0.Id)->Balance);

    const Order order2 = Order::Buy(2, test_symbol.Id, test_user1.Id, 200, 100);
    market_manager.AddOrder(order2);

    EXPECT_EQ(0, market_manager.GetOrderBook(test_symbol.Id)->size());
    EXPECT_EQ(nullptr, market_manager.GetOrderBook(test_symbol.Id)->best_bid());
    EXPECT_EQ(nullptr, market_manager.GetOrderBook(test_symbol.Id)->best_ask());
    EXPECT_EQ(200 * 100, market_manager.GetUser(test_user0.Id)->Balance);
    EXPECT_EQ(-(200 * 100), market_manager.GetUser(test_user1.Id)->Balance);
}

TEST_F(MarketManagerTradingTest, UnfilledOrderMatchingSell) {
    const Order order = Order::Sell(1, test_symbol.Id, test_user0.Id, 200, 100);
    market_manager.AddOrder(order);

    EXPECT_EQ(1, market_manager.GetOrderBook(test_symbol.Id)->size());
    EXPECT_EQ(order.Quantity, market_manager.GetOrderBook(test_symbol.Id)->best_ask()->TotalVolume);
    EXPECT_EQ(order.Price, market_manager.GetOrderBook(test_symbol.Id)->best_ask()->Price);
    EXPECT_EQ(0, market_manager.GetUser(test_user0.Id)->Balance);

    // TODO: need something to do with auto delete without manually call
    // The container only stores a reference and that you must make sure that the inserted elements stay alive longer than the container.
    market_manager.DeleteOrder(order.Id);
}

TEST_F(MarketManagerTradingTest, PartialFillWithCancelledOrderSell) {
    const Order order1 = Order::Sell(1, test_symbol.Id, test_user0.Id, 200, 100);
    market_manager.AddOrder(order1);

    const Order order2 = Order::Buy(2, test_symbol.Id, test_user1.Id, 210, 50);
    market_manager.AddOrder(order2);

    market_manager.DeleteOrder(order1.Id);

    EXPECT_EQ(0, market_manager.GetOrderBook(test_symbol.Id)->size());
    EXPECT_EQ(nullptr, market_manager.GetOrderBook(test_symbol.Id)->best_bid());
    EXPECT_EQ(nullptr, market_manager.GetOrderBook(test_symbol.Id)->best_ask());
    EXPECT_EQ(200 * 50, market_manager.GetUser(test_user0.Id)->Balance);
    EXPECT_EQ(-(200 * 50), market_manager.GetUser(test_user1.Id)->Balance);

    // TODO: need something to do with auto delete without manually call
    // The container only stores a reference and that you must make sure that the inserted elements stay alive longer than the container.
    market_manager.DeleteOrder(order1.Id);
}
