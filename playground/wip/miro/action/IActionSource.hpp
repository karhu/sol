#pragma once

#include <vector>

#include "ActionRange.hpp"

namespace miro { namespace action {

    class IActionSink;

    class IActionSource
    {
    protected:
        void send(ActionRange action_range);
    protected:
        virtual bool on_connect(IActionSink& sink);
        virtual bool on_disconnect(IActionSink& sink);
    private:
        bool add_sink(IActionSink& sink);
        bool remove_sink(IActionSink& sink);
    private:
        std::vector<IActionSink*> m_sinks;
    private:
        friend bool connect(IActionSource &source, IActionSink &sink);
        friend bool disconnect(IActionSource &source, IActionSink &sink);
        friend class IActionSink;
    };

}}
