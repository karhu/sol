#pragma once

namespace sol {
namespace memory {

class AAllocator;

template<typename T>
struct rich_ptr
{
public:
    rich_ptr() {}
    rich_ptr(const rich_ptr& other) = default;
public:
    void destroy();
public:
    inline T* operator->() { return m_ptr; }
    inline const T* operator->() const { return m_ptr; }
public:
    inline T* raw() { return m_ptr; }
    AAllocator *allocator();
private:
    rich_ptr(T* ptr) : m_ptr(ptr) {}
private:
    T* m_ptr = nullptr;
private:
    friend class AAllocator;
};

template<typename T>
void rich_ptr<T>::destroy()
{
    if (m_ptr == nullptr) return;
    raw()->~T();
    allocator()->free(raw());
}

}}
