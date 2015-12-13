#pragma once

namespace miro { namespace action {

    class IActionSource;
    class IActionSink;

    bool connect(IActionSource& source, IActionSink& sink);
    bool disconnect(IActionSource& source, IActionSink& sink);

}}
