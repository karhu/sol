#pragma once

#include  <type_traits>

namespace arc {

    template<typename T>
    struct Result {
    private:
        using StorageT = typename std::aligned_storage<sizeof(T), alignof(T)>::type;
    public:
        Result(T&& value)
            : m_valid(true)
        {
            ::new(&m_storage) T(std::move(value));
        }

        Result()
            : m_valid(false)
        {}

        ~Result() {
            destroy();
        }

    public:
        bool valid() const { return m_valid; }

        T& value() {
            //TODO: check for valid
            return *value_ptr();
        }

        const T& value() const {
            //TODO: check for valid
            return *value_ptr();
        }

        T&& unwrap() {
            //TODO: check for valid?
            m_valid = false;
            return std::move(*value_ptr());
        }
    private:
        void destroy() {
            //TODO only if not a reference
            if (m_valid) value_ptr()->~T();
        }

        T* value_ptr() { return (T*)&m_storage; }

        bool m_valid;
        StorageT m_storage;
    };

}
