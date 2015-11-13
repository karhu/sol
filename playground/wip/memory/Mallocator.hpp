#pragma once

#include "AAllocator.hpp"

namespace sol {
namespace memory {

class Mallocator : public AAllocator {
protected:
    void* raw_allocate(uint64_t size) override;
    void  raw_free(void *ptr, uint64_t size) override;
};

}} // sol::memory
