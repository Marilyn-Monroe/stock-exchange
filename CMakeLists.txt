cmake_minimum_required(VERSION 3.14)
project(stock_exchange CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost COMPONENTS system REQUIRED)
find_package(Threads REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})

add_subdirectory(third_party/nlohmann_json)

add_library(${PROJECT_NAME}_objs OBJECT
        src/common.hpp
        src/errors.hpp
        src/level.hpp
        src/market_manager.cpp
        src/market_manager.hpp
        src/order.hpp
        src/order_book.cpp
        src/order_book.hpp
        src/symbol.hpp
        src/update.hpp
        src/user.hpp
)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_objs Threads::Threads ${Boost_LIBRARIES} nlohmann_json)

enable_testing()
find_package(GTest REQUIRED)

add_executable(${PROJECT_NAME}_unittest
        tests/test_symbol.cpp
        tests/test_orderbook.cpp
        tests/test_order.cpp
        tests/test_user.cpp
        tests/test_trading.cpp
)
target_link_libraries(${PROJECT_NAME}_unittest PRIVATE ${PROJECT_NAME}_objs GTest::gtest_main)

gtest_discover_tests(${PROJECT_NAME}_unittest)
