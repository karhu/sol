#pragma once


namespace sol {
namespace memory {

inline uint64_t align(uint64_t address, uint64_t alignment) {
    auto p = address;
    auto a = alignment;
    auto p2 = (1 + (p - 1) / a) * a;
    return p2;
}

}} // sol::memory
