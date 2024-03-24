#include <boost/cstdint.hpp>

constexpr boost::uint16_t PORT = 5555;

enum class Requests : boost::uint64_t {
    Registration,
    ViewBalance,
    AddOrder
};
