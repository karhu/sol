#pragma once

#include "common.hpp"
#include "sol/MemoryRange.hpp"

namespace miro { namespace action {

    class ActionBuffer;
    struct ActionHeader;

    struct ActionRef
    {
    public:
        inline ActionRef(ActionBuffer& buffer, uint16_t index)
            : m_buffer(&buffer)
            , m_index(index)
        {}
    public:
        const ActionHeader& header() const;
        sol::MemoryRange data_memory();

        template<typename AT>
        AT data() { return AT(*m_buffer,m_index); }

    private:
        ActionBuffer* m_buffer = nullptr;
        uint16_t m_index = 0;
    };

    struct ActionRange
    {
    public:
        inline uint16_t count() const { return m_end - m_begin; }

        inline ActionRef get(uint16_t idx) {
            // TODO index assert
            return ActionRef(*m_buffer,idx);
        }
    public:
        inline ActionBuffer& buffer() { return *m_buffer; }
        inline uint16_t begin_index() { return m_begin; }
        inline uint16_t end_index() { return m_end; }
    public:
        inline ActionRange(ActionBuffer& buffer, uint16_t begin, uint16_t end)
            : m_buffer(&buffer)
            , m_begin(begin)
            , m_end(end)
        {}
    private:
        ActionBuffer* m_buffer = nullptr;
        uint16_t m_begin = 0;
        uint16_t m_end = 0;
    };

}}
