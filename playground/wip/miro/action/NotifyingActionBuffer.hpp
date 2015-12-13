#pragma once

#include "delegate.hpp"
#include "BufferingActionSink.hpp"

namespace miro { namespace action {

class NotifyingActionBuffer : public BufferingActionSink {
public:
    void set_notify_callback(sol::delegate<void()> cb);
protected:
    virtual void on_receive(action::ActionRange range) override;
private:
    sol::delegate<void()>  m_notify_cb = nullptr;
};

}}
