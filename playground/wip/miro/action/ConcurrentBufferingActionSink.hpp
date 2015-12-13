#pragma once

#include <mutex>

#include "delegate.hpp"
#include "BufferingActionSink.hpp"

namespace miro { namespace action {

    class ConcurrentBufferingActionSink : public BufferingActionSink
    {
    public:
        template<typename CB>
        void handle_actions(CB&& cb); // CB: (actions::ActionRange) -> bool
    public:
        void set_notify_callback(sol::delegate<void()> cb);
    protected:
        virtual void on_receive(action::ActionRange range) override;
    protected:
        std::mutex m_mutex;
        sol::delegate<void()>  m_notify_cb = nullptr;
    };

    template<typename CB>
    void ConcurrentBufferingActionSink::handle_actions(CB &&cb)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        BufferingActionSink::handle_actions(cb);
    }

}}
