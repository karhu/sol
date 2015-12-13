#pragma once

#include "sol/StringView.hpp"
#include "RelativeMemoryRange16.hpp"
#include "StringRef.hpp"

namespace miro { namespace action {

    template<typename DataT>
    struct ActionReference
    {
    public:
        ActionReference() {}
        ActionReference(ActionBuffer& buffer, uint16_t action_idx) {
            if (action_idx >= buffer.m_headers.size()) return;
            auto type = buffer.m_headers[action_idx].meta.type;
            if (type != Type) return;

            m_buffer = &buffer;
            m_action_idx = action_idx;
            m_mem_ref = buffer.m_headers[action_idx].memory;
            m_data = (DataT*)buffer.get_memory(m_mem_ref).begin();
        }
    public:
        static constexpr ActionType Type = DataT::Type;
    public:
        bool valid() { return m_buffer != nullptr; }
        const HeaderData& header() { return m_buffer->m_headers[m_action_idx].meta;}
    protected:
        RelativeMemoryRange16 mem_ref() { return m_mem_ref; }
        DataT& data() { return *m_data; }
        sol::StringView resolve_string(StringRef ref) { return ref.read(*m_buffer, m_mem_ref); }
    private:
        ActionBuffer* m_buffer = nullptr;
        DataT* m_data = nullptr;
        uint16_t m_action_idx = 0;
        RelativeMemoryRange16 m_mem_ref;
    };

}}
