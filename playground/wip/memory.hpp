#pragma once

#include <cstdint>
#include <algorithm>
#include <memory>

namespace sol {

class IAllocator;

template<typename T>
struct rich_ptr
{
public:
    rich_ptr() {}
    rich_ptr(const rich_ptr& other) = default;
public:
    void destroy();
public:
    inline T& operator->() { return *m_ptr; }
    inline const T& operator->() const { return *m_ptr; }
public:
    inline T* raw() { return m_ptr; }
    IAllocator* allocator();
private:
    rich_ptr(T* ptr) : m_ptr(ptr) {}
private:
    T* m_ptr = nullptr;
private:
    friend class IAllocator;
};

inline uint64_t align(uint64_t address, uint64_t alignment) {
    auto p = address;
    auto a = alignment;
    auto p2 = (1 + (p - 1) / a) * a;
    return p2;
}

class IAllocator
{
public:
    struct Header
    {
        uint64_t get_size() { return m_size*IAllocator::ALLOCATION_UNIT; }
        void set_size(uint64_t size) { m_size = size/IAllocator::ALLOCATION_UNIT; }
        uint32_t get_alignment() { return m_alignment; }
        void set_alignment(uint32_t a) { m_alignment = a; }
    public:
        uint32_t allocator_index;
        uint16_t offset;
    private:
        uint16_t m_alignment; //TODO
        uint32_t m_size; //38
    };
    static const uint64_t MIN_ALIGNMENT = 8;
    static const uint64_t ALLOCATION_UNIT = 4;
public:
    IAllocator();
    virtual ~IAllocator();
    IAllocator(const IAllocator& other) = delete;
    IAllocator& operator=(const IAllocator& other) = delete;
public:
    template<typename T, typename ...Args>
    rich_ptr<T> create(Args&&... args) {
        const uint64_t min_alignment = IAllocator::MIN_ALIGNMENT;
        const uint64_t alignment = std::max(min_alignment,(uint64_t)alignof(T));
        const uint64_t size = sizeof(T);
        const uint64_t header_size = sizeof(Header);
        const uint64_t extra = header_size + alignment/2;
        // make allocation size a multiple of ALLOCATION_UNIT
        const uint64_t alloc_size = align(size+extra, ALLOCATION_UNIT);
        const auto ptr = (uint64_t) raw_allocate(alloc_size);
        const auto ptr_aligned = (uint64_t) align(ptr+header_size,alignment);

        const auto header = (Header*)(ptr_aligned-header_size);
        header->allocator_index = m_index;
        header->set_size(alloc_size);
        header->offset = ptr_aligned-ptr;
        header->set_alignment(alignment);

        T* t = new((void*)ptr_aligned) T(args...);
        return rich_ptr<T>(t);
    }

    void free(void* ptr) {
        auto p = (uint8_t*) ptr;
        const auto header = (Header*)(p - sizeof(Header));
        raw_free(p - header->offset, header->get_size());
    }

protected:
    static IAllocator* get_allocator(uint32_t index);
    static std::vector<IAllocator*> s_allocators;
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
IAllocator* rich_ptr<T>::allocator()
{
    if (m_ptr == nullptr) nullptr;
    auto ptr = (uint8_t*) raw();
    const auto header = (IAllocator::Header*)(ptr - sizeof(IAllocator::Header));
    return IAllocator::get_allocator(header->allocator_index);
}

template<typename T>
void rich_ptr<T>::destroy()
{
    if (m_ptr == nullptr) return;
    raw()->~T();
    allocator()->free(raw());
}

class Mallocator : public IAllocator {
protected:
    void* raw_allocate(uint64_t size) override;
    void  raw_free(void *ptr, uint64_t size) override;
};

}
