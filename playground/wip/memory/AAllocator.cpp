#include "AAllocator.hpp"

namespace sol {
namespace memory {

AAllocator::AAllocator()
{
    s_allocators.push_back(this);
    m_index = s_allocators.size() - 1;
}

AAllocator::~AAllocator()
{
    s_allocators[m_index] = nullptr;
}

AAllocator *AAllocator::get_allocator(uint32_t index)
{
    if (index < s_allocators.size()) return s_allocators[index];
    else return nullptr;
}

std::vector<AAllocator*> AAllocator::s_allocators(1,nullptr);

}} // sol::memory
