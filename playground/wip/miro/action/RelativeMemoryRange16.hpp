#pragma once

#include "common.hpp"

namespace miro { namespace action {

    struct RelativeMemoryRange16 {
        uint16_t m_offset = 0;
        uint16_t m_size = 0;

        bool valid() const { return m_size > 0; }
        uint16_t size() const { return m_size; }

        void shorten_front(uint16_t amount) {
            if (amount > m_size){
                m_offset += m_size;
                m_size = 0;
            } else {
                m_offset += amount;
                m_size -= amount;
            }
        }
    };

}}
