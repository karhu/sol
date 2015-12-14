#include "BufferingActionSink.hpp"

namespace miro { namespace action {

    BufferingActionSink::BufferingActionSink()
        : m_current_buffer(new ActionBuffer())
    {}

    void BufferingActionSink::on_receive(ActionRange range)
    {
        auto count = range.count();
        while (true) {
            range = m_current_buffer->copy_action(range);
            if (range.count() == 0 ) break;
            end_current_write_buffer();
        }
        m_count += count;
    }

    void BufferingActionSink::end_current_write_buffer()
    {
        if (m_current_buffer->count() == 0) return;

        m_full_buffers.push_back(std::move(m_current_buffer));
        if (m_available_buffers.empty())
            m_current_buffer.reset(new ActionBuffer());
        else {
            m_current_buffer = std::move(m_available_buffers.back());
            m_available_buffers.pop_back();
        }
    }

}}
