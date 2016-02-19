#pragma once

#include "../common.hpp"

namespace sol
{
    template<typename T>
    struct Slice
    {
    public:
        using ElementType = T;
    public:
        constexpr Slice(T* data = nullptr, uint64_t size = 0);
    public:
        /// element access
        T& operator[] (uint64_t idx);

        /// element access
        const T& operator[] (uint64_t idx) const;

        /// raw pointer
        T* ptr();

        /// raw pointer
        const T* ptr() const;

        /// element count
        uint64_t count() const;
    public:
        void trim_front(uint32_t n);
        void trim_back(uint32_t n);

    private:
        T* _data = nullptr;
        uint64_t _size = 0;
    };

        // implementation ////////////////////////////////////////////////////////////////

    template<typename T> inline
    constexpr Slice<T>::Slice(T* data, uint64_t size) : _data(data), _size(size) {}

    template<typename T> inline
    constexpr Slice<T> make_slice(T* data, uint64_t size)
    {
        return Slice<T>(data,size);
    }

    template<typename T> inline
    constexpr const Slice<T> make_slice(const T* begin, const T* end)
    {
        return Slice<T>(const_cast<T*>(begin), end - begin);
    }

    template<typename T> inline
    constexpr Slice<T> make_slice(T* begin, T* end)
    {
        return Slice<T>(begin, end - begin);
    }

    template<typename T> inline
    constexpr const Slice<T> make_slice(const T* data, uint64_t size)
    {
        return Slice<T>(const_cast<T*>(data), size);
    }

    template<typename T> inline
    T& Slice<T>::operator[] (uint64_t idx)
    {
        return _data[idx];
    }

    template<typename T> inline
    const T& Slice<T>::operator[] (uint64_t idx) const
    {
        return _data[idx];
    }

    template<typename T> inline
    T* Slice<T>::ptr()
    {
        return _data;
    }

    template<typename T> inline
    const T* Slice<T>::ptr() const
    {
        return _data;
    }

    template<typename T> inline
    uint64_t Slice<T>::count() const
    {
        return _size;
    }

    template<typename T> inline
    void Slice<T>::trim_front(uint32_t n)
    {
        //ARC_ASSERT(n <= _size, "Trim value larger than buffer length");
        _data += n;
        _size -= n;
    }

    template<typename T> inline
    void Slice<T>::trim_back(uint32_t n)
    {
        //ARC_ASSERT(n <= _size, "Trim value larger than buffer length");
        _size -= n;
    }


    // range based for-loop support /////////////////////////////////////////////////////

    template<typename T> constexpr
    T* begin(sol::Slice<T>& a) { return a.ptr(); }

    template<typename T> constexpr
    T* end (sol::Slice<T>& a) { return a.ptr() + a.count(); }

    template<typename T> constexpr
    const T* begin(const sol::Slice<T>& a) { return a.ptr(); }

    template<typename T> constexpr
    const T* end (const sol::Slice<T>& a) { return a.ptr() + a.count(); }

} // namespace std
