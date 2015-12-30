#pragma once

#include <stddef.h>

namespace sol {

    struct StringView
    {
        StringView() {}
        StringView(char* front, size_t size)
            : m_front(front), m_end(front+size)
        {}

        size_t size() const { return m_end - m_front; }
        char* ptr() { return m_front; }
        const char* ptr() const { return m_front; }
    private:
        char* m_front = nullptr;
        char* m_end = nullptr;
    };

}
