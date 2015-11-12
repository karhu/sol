#include "memory.hpp"

namespace sol {

void* Mallocator::raw_allocate(uint64_t size)
{
    return std::malloc(size);
}

void Mallocator::raw_free(void *ptr, uint64_t size)
{
    return std::free(ptr);
}

IAllocator::IAllocator()
{
    s_allocators.push_back(this);
    m_index = s_allocators.size() - 1;
}

IAllocator::~IAllocator()
{
    s_allocators[m_index] = nullptr;
}

IAllocator *IAllocator::get_allocator(uint32_t index)
{
    if (index < s_allocators.size()) return s_allocators[index];
    else return nullptr;
}

std::vector<IAllocator*> IAllocator::s_allocators(1,nullptr);

}

