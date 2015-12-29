#pragma once

#include <cstring>
#include <vector>

#include "sol/MemoryRange.hpp"

#include "RelativeMemoryRange16.hpp"
#include "ActionType.hpp"
#include "ActionHeader.hpp"
#include "ActionRange.hpp"

namespace miro { namespace action {

    class ActionBuffer;

    struct RelativeMemoryRange16Writer
    {
        RelativeMemoryRange16Writer() {}
        RelativeMemoryRange16Writer(ActionBuffer& b, RelativeMemoryRange16 memory);

        template<typename T, typename ...Args>
        T* emplace(Args&& ...args) {
            auto size = sizeof(T);
            if (available() < size) return nullptr;
            T* ptr = new(front_ptr()) T(std::forward<Args>(args)...);
            move_front(size);
            return ptr;
        }

        RelativeMemoryRange16 relative_frame() const { return m_memory_range; }
        uint16_t available() const { return m_memory_range.size() - m_front; }
        uint16_t front() const { return m_front; }
        void* front_ptr() const { return m_memory_front; }
        void  move_front(uint16_t v) {
            m_memory_front = ((char*) m_memory_front) + v;
            m_front += v;
        }
        bool valid() const { return m_memory_range.valid(); }
    private:
        RelativeMemoryRange16 m_memory_range;
        uint16_t m_front = 0;
        void* m_memory_front = nullptr;
    };

    class ActionBuffer {
    public:
        std::vector<ActionHeader> m_headers;
        std::vector<uint8_t> m_data;
        uint16_t m_front = 0;

        uint16_t available() const { return DATA_SIZE - m_front; }
        uint16_t count() const { return m_headers.size(); }
        uint16_t size_headers() const { return m_headers.size() * sizeof(ActionHeader); }
        uint16_t size_data() const { return m_front; }

        uint8_t const* ptr_data() const { return &m_data[0]; }
        ActionHeader const * ptr_headers() const { return &m_headers[0]; }

        void reset() {
            m_front = 0;
            m_data.resize(DATA_SIZE);
            m_headers.clear();

            for (auto& d : m_data) d = 0; // TODO remove
        }

        static constexpr uint16_t DATA_SIZE = 65535u; // uint16::max
    public:
        ActionBuffer()
            : m_data(DATA_SIZE)
        {}
    public:
        RelativeMemoryRange16Writer begin_action(uint16_t size)
        {
            if (available() < size) {
                return RelativeMemoryRange16Writer();
            }

            RelativeMemoryRange16 range;
            range.m_offset = m_front;
            range.m_size = available();
            return RelativeMemoryRange16Writer(*this,range);
        }

        template<typename ActionT>
        void end_action(RelativeMemoryRange16Writer& writer, HeaderMeta hm)
        {
            m_headers.emplace_back();
            auto& h = m_headers.back();
            h.memory = writer.relative_frame();
            h.memory.m_size = writer.front();
            h.meta.user = hm.user;
            h.meta.timestamp = hm.timestamp;
            h.meta.type = ActionT::Type;

            m_front = writer.relative_frame().m_offset + writer.front();
        }

        ActionHeader* allocate_action(size_t data_size)
        {
            // check if we have enough memory available
            if (available() < data_size) {
                return nullptr;
            }
            ActionHeader* result;

            m_headers.emplace_back();
            result = &m_headers.back();
            result->memory.m_size = (uint16_t)data_size;
            result->memory.m_offset = m_front;
            m_front += data_size;

            return result;
        }

        sol::MemoryRange get_memory(RelativeMemoryRange16 range)
        {
            return sol::MemoryRange(m_data.data()+range.m_offset,range.m_size);
        }

        bool copy_action(ActionBuffer source, uint16_t source_idx);

        ActionRange copy_action(ActionRange range);

    public:
        inline ActionType read_action_type(uint16_t action_offset)
        {
            if (action_offset >= m_front)
                return ActionType::Unknown;
            else
                return m_headers[action_offset].meta.type;
        }

        inline ActionRange all() {
            return ActionRange(*this,0,m_headers.size());
        }
    };


    // implementation ////////////////////////

    inline RelativeMemoryRange16Writer::RelativeMemoryRange16Writer(ActionBuffer& b, RelativeMemoryRange16 memory)
        : m_memory_range(memory)
        , m_memory_front(b.get_memory(memory).begin())
    {}

}}
