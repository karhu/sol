#include "ConcurrentBufferingActionSink.hpp"

namespace miro { namespace action {

    void ConcurrentBufferingActionSink::set_notify_callback(sol::delegate<void ()> cb)
    {
        m_notify_cb = cb;
    }

    void ConcurrentBufferingActionSink::on_receive(ActionRange range)
    {
        auto count = range.count();
        {
        std::lock_guard<std::mutex> lock(m_mutex);
        BufferingActionSink::on_receive(range);
        }
        if (count && m_notify_cb) m_notify_cb();
    }

}}



