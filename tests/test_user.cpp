#include <gtest/gtest.h>

#include "../src/market_manager.hpp"

class MarketManagerUserTest : public ::testing::Test {
protected:
    MarketManager market_manager;
    const User test_user{0, "user"};

    void SetUp() override {

    }

    void TearDown() override {

    }
};

TEST_F(MarketManagerUserTest, AddUserTest) {
    EXPECT_EQ(ErrorCode::OK, market_manager.AddUser(test_user));

    EXPECT_EQ(ErrorCode::USER_DUPLICATE, market_manager.AddUser(test_user));
}

TEST_F(MarketManagerUserTest, DeleteUserTest) {
    market_manager.AddUser(test_user);

    EXPECT_EQ(ErrorCode::OK, market_manager.DeleteUser(test_user.Id));

    EXPECT_EQ(ErrorCode::USER_NOT_FOUND, market_manager.DeleteUser(test_user.Id));
}

TEST_F(MarketManagerUserTest, GetUserTest) {
    market_manager.AddUser(test_user);

    EXPECT_EQ(test_user.Id, market_manager.GetUser(test_user.Id)->Id);

    const User non_existent_user(1, "user1");
    EXPECT_EQ(nullptr, market_manager.GetUser(non_existent_user.Id));
}
