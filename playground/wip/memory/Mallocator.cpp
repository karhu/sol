#include "Mallocator.hpp"

#include <memory>

#include "../common.hpp"

namespace sol {
namespace memory {

void* Mallocator::raw_allocate(uint64_t size)
{
    return std::malloc(size);
}

void Mallocator::raw_free(void *ptr, uint64_t size)
{
    UNUSED(size);
    return std::free(ptr);
}

}} // sol::memory
