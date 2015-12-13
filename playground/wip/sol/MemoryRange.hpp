#pragma once

namespace sol {

    struct MemoryRange {
    public:
        MemoryRange(void* begin = nullptr, size_t size = 0) : m_begin(begin), m_end((char*)begin+size) {}
        ~MemoryRange() = default;

    public:
        void* begin() const { return m_begin; }
        void* end() const { return m_end; }
        size_t size() const { return (char*)m_end - (char*)m_begin; }
    private:
        void* m_begin = 0;
        void* m_end = 0;
    };

}
