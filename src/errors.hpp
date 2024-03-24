#pragma once

#include <boost/cstdint.hpp>

enum class ErrorCode : boost::uint8_t {
    OK,
    SYMBOL_DUPLICATE,
    SYMBOL_NOT_FOUND,
    ORDER_BOOK_DUPLICATE,
    ORDER_BOOK_NOT_FOUND,
    ORDER_DUPLICATE,
    ORDER_NOT_FOUND,
    ORDER_ID_INVALID,
    ORDER_QUANTITY_INVALID,
    USER_DUPLICATE,
    USER_NOT_FOUND
};
