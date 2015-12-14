#pragma once

#include "IActionSource.hpp"
#include "ActionBuffer.hpp"

#include <iostream>

namespace miro { namespace action {

    class ActionWriter : public IActionSource {
    public:
        action::ActionBuffer& buffer() { return m_buffer; }
    public:
        uint16_t count_written() const { return m_buffer.count(); }

        void reset() {
            m_buffer.reset();
        }

        void send_and_reset() {
            send(m_buffer.all());
            reset();
        }
    private:
        action::ActionBuffer m_buffer;
    };


}}
