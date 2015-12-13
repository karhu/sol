#pragma once

#include <vector>

#include "ActionRange.hpp"

namespace miro { namespace action {

    class IActionSource;

    class IActionSink
    {
    protected:
        virtual bool on_connect(IActionSource& source);
        virtual bool on_disconnect(IActionSource& source);
        virtual void on_receive(ActionRange action_range);
    private:
        void receive(ActionRange action_range);
    private:
        bool add_source(IActionSource& sink);
        bool remove_source(IActionSource& sink);
    private:
        std::vector<IActionSource*> m_sources;
    private:
        friend bool connect(IActionSource &source, IActionSink &sink);
        friend bool disconnect(IActionSource &source, IActionSink &sink);
        friend class IActionSource;
    };

}}
