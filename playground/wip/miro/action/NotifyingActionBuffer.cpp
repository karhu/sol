#include "NotifyingActionBuffer.hpp"

namespace miro { namespace action {

    void NotifyingActionBuffer::set_notify_callback(sol::delegate<void ()> cb)
    {
        m_notify_cb = cb;
    }

    void NotifyingActionBuffer::on_receive(ActionRange range)
    {
        BufferingActionSink::on_receive(range);
        if (m_notify_cb) m_notify_cb();
    }

}}

