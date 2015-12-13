#pragma once

#include "IActionSink.hpp"
#include "IActionSource.hpp"

namespace miro { namespace action {

    class ActionForwarder : public IActionSink, public IActionSource
    {
    protected:
        virtual void on_receive(ActionRange range) override;
    };

}}
