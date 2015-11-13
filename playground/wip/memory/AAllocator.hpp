#pragma once

#include <cstdint>
#include <vector>

#include "rich_ptr.hpp"
#include "util.hpp"

namespace sol {
namespace memory {

class AAllocator
{
public:
    struct Header
    {
        uint64_t get_size() { return m_req_size*AAllocator::ALLOCATION_UNIT; }
        void set_size(uint64_t size) { m_req_size = size/AAllocator::ALLOCATION_UNIT; }
        uint32_t get_alignment() { return m_alignment; }
        void set_alignment(uint32_t a) { m_alignment = a; }
    public:
        uint32_t allocator_index;
        uint16_t offset;
    private:
        uint16_t m_alignment; //TODO
        uint32_t m_req_size; //38
    };
    static const uint64_t MIN_ALIGNMENT = 8;
    static const uint64_t ALLOCATION_UNIT = 4;
public:
    AAllocator();
    virtual ~AAllocator();
    AAllocator(const AAllocator& other) = delete;
    AAllocator& operator=(const AAllocator& other) = delete;
public:
    static uint64_t allocated_size(uint64_t req_size, uint64_t alignment) {
        const uint64_t header_size = sizeof(Header);
        const uint64_t extra = header_size + alignment/2;
        // make allocation size a multiple of ALLOCATION_UNIT
        return align(req_size+extra, ALLOCATION_UNIT);
    }

    template<typename T, typename ...Args>
    rich_ptr<T> create(Args&&... args) {
        const uint64_t req_alignment = (uint64_t)alignof(T);
        const uint64_t req_size = sizeof(T);

        const uint64_t header_size = sizeof(Header);
        const uint64_t min_alignment = AAllocator::MIN_ALIGNMENT;
        const uint64_t alignment = std::max(min_alignment,req_alignment);

        const uint64_t alloc_size = allocated_size(req_size, alignment);
        const auto ptr = (uint64_t) raw_allocate(alloc_size);
        const auto ptr_aligned = align(ptr+header_size,alignment);

        const auto header = (Header*)(ptr_aligned-header_size);
        header->allocator_index = m_index;
        header->set_size(req_size);
        header->offset = ptr_aligned-ptr;
        header->set_alignment(alignment);

        T* t = new((void*)ptr_aligned) T(args...);
        return rich_ptr<T>(t);
    }

    void free(void* ptr) {
        auto p = (uint8_t*) ptr;
        const auto header = (Header*)(p - sizeof(Header));
        const auto alloc_size = allocated_size(header->get_size(),header->get_alignment());
        raw_free(p - header->offset, alloc_size);
    }

protected:
    static AAllocator* get_allocator(uint32_t index);
    static std::vector<AAllocator*> s_allocators;
protected:
    virtual void* raw_allocate(uint64_t size) = 0;
    virtual void  raw_free(void* ptr, uint64_t size) = 0;
private:
    uint32_t m_index;
private:
    template<typename T>
    friend struct rich_ptr;
};

template<typename T>
AAllocator* rich_ptr<T>::allocator()
{
    if (m_ptr == nullptr) return nullptr;
    auto ptr = (uint8_t*) raw();
    const auto header = (AAllocator::Header*)(ptr - sizeof(AAllocator::Header));
    return AAllocator::get_allocator(header->allocator_index);
}

}} // sol::memory
