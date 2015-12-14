#pragma once

#include "common.hpp"
#include "sol/StringView.hpp"
#include "RelativeMemoryRange16.hpp"

namespace miro { namespace action {

    class RelativeMemoryRange16Writer;
    class ActionBuffer;

    struct StringRef {
        int16_t m_local_offset = 0;
        int16_t m_size = 0;

        bool write(RelativeMemoryRange16Writer& writer, const std::string& str);
        sol::StringView read(ActionBuffer& b, RelativeMemoryRange16 mem_ref);
    };

}}
