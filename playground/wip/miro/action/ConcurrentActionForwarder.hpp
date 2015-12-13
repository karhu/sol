#pragma once

#include "IActionSource.hpp"
#include "ConcurrentBufferingActionSink.hpp"

namespace miro { namespace action {

    class ConcurrentActionForwarder
            : public ConcurrentBufferingActionSink
            , public IActionSource
    {
    public:
        void poll(); // to be called from the thread that manages connected sinks
    };

}}
