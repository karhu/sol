#pragma once

#include "common.hpp"
#include "sol/StringView.hpp"

namespace miro { namespace action {

    struct StringRef {
        int16_t m_local_offset = 0;
        int16_t m_size = 0;

        bool write(RelativeMemoryRange16Writer& writer, const std::string& str)
        {
            auto str_size = str.size() + 1;
            if (writer.available() < str_size) return false;
            std::memcpy(writer.front_ptr(),str.c_str(),str_size);
            m_local_offset = writer.front();
            m_size = str_size;
            writer.move_front(str_size);
            return true;
        }

        sol::StringView read(ActionBuffer& b, RelativeMemoryRange16 mem_ref)
        {
            mem_ref.shorten_front(m_local_offset);
            if (m_size > mem_ref.m_size) return sol::StringView();
            auto mem = b.get_memory(mem_ref);
            return sol::StringView((char*)mem.begin(), m_size);
        }

    };

}}
