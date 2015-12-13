#include "ActionRange.hpp"

#include "ActionBuffer.hpp"

namespace miro { namespace action {

    const ActionHeader& ActionRef::header() const
    {
        return m_buffer->m_headers[m_index];
    }

    sol::MemoryRange ActionRef::data_memory()
    {
        return m_buffer->get_memory(header().memory);
    }

}}
