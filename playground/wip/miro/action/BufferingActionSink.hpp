#pragma once

#include <deque>
#include "IActionSink.hpp"
#include "ActionBuffer.hpp"

namespace miro { namespace action {

    class BufferingActionSink : public IActionSink {
    public:
        BufferingActionSink();
    public:
        template<typename CB>
        void handle_actions(CB&& cb); // CB: (actions::ActionRange) -> bool

        uint32_t count() const { return m_count; }
    protected:
        virtual void on_receive(action::ActionRange range) override;
    protected:
        void end_current_write_buffer();
    protected:
        std::unique_ptr<action::ActionBuffer> m_current_buffer;
        std::deque<std::unique_ptr<action::ActionBuffer>> m_full_buffers;
        std::vector<std::unique_ptr<action::ActionBuffer>> m_available_buffers;
        uint32_t m_count = 0;
    };

    template<typename CB>
    void BufferingActionSink::handle_actions(CB &&cb)
    {
        end_current_write_buffer();

        // handle the ranges
        while (!m_full_buffers.empty()) {
            auto buffer = std::move(m_full_buffers.front());
            m_full_buffers.pop_front();
            auto range = buffer->all();
            m_count -= range.count();
            bool cont = cb(range);
            buffer->reset();
            m_available_buffers.push_back(std::move(buffer));
            if (!cont) break;
        }
    }

}}
