#pragma once

namespace miro { namespace action {

    class ActionSender : public IActionSource {
    public:
        inline void send(action::ActionRange action_range) { IActionSource::send(action_range); }
    };

}}
