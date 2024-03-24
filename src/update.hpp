#pragma once

#include <boost/cstdint.hpp>

enum class UpdateType : boost::uint8_t {
    NONE,
    ADD,
    UPDATE,
    DELETE
};
