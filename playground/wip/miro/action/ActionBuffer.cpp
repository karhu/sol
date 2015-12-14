#include "ActionBuffer.hpp"

#include "ActionDefinitions.hpp"
#include <iostream>

namespace miro { namespace action {

    bool ActionBuffer::copy_action(ActionBuffer source, uint16_t source_idx)
    {
        auto h_read = source.m_headers[source_idx];

        auto h_write = h_read;
        h_write.memory.m_offset = m_front;

        auto mr = source.get_memory(h_read.memory);
        auto size = mr.size();
        if (available() < size) return false;

        void* dest_begin = &m_data[m_front];
        std::memcpy(dest_begin,mr.begin(),size);
        m_front += mr.size();
        m_headers.push_back(h_write);
        return true;
    }

    ActionRange ActionBuffer::copy_action(ActionRange range) {
        for (uint16_t i = 0; i<range.count(); i++) {
            uint16_t idx = range.begin_index() + i;
            //float p = range.get(i).data<StrokeActionRef>().position().x;
            //std::cout << "cp " << p <<std::endl;
            if (!copy_action(range.buffer(),idx)) {
                return ActionRange(range.buffer(), idx, range.end_index());
            }
            //StrokeActionRef ref(*this,count()-1);
            //std::cout << "cp2 " << count() -1 << " " << ref.position().x <<std::endl;
        }
        return ActionRange(range.buffer(),0,0);
    }

}}

