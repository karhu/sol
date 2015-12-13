#pragma once

#include "IActionSource.hpp"
#include "ActionBuffer.hpp"

namespace miro { namespace action {

    class ActionWriter : public IActionSource {
    public:
        action::ActionBuffer& buffer() { return m_buffer; }
    public:
        uint16_t count_sent() const { return m_sent_front; }
        uint16_t count_written() const { return m_buffer.count(); }
        uint16_t count_ready() const { return count_written() - count_sent(); }

        void reset() {
            m_buffer.reset();
            m_sent_front = 0;
        }

        void send_and_reset() {
            send(action::ActionRange(m_buffer,count_sent(),count_written()));
            reset();
        }
    private:
        action::ActionBuffer m_buffer;
        uint16_t m_sent_front = 0;
    };


}}
