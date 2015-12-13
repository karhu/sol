#include "connect.hpp"

#include "IActionSink.hpp"
#include "IActionSource.hpp"

namespace miro { namespace action {

    bool connect(IActionSource &source, IActionSink &sink)
    {
        bool ok1, ok2;
        ok1 = sink.add_source(source);
        if (!ok1) {
            return false;
        }
        ok2 = source.add_sink(sink);
        if (!ok2) {
            sink.remove_source(source);
            return false;
        }

        ok1 = sink.on_connect(source);
        if (!ok1) {
            source.remove_sink(sink);
            sink.remove_source(source);
            return false;
        }

        ok2 = source.on_connect(sink);
        if (!ok2) {
            sink.on_disconnect(source);
            source.remove_sink(sink);
            sink.remove_source(source);
        }
        return true;
    }


    bool disconnect(IActionSource &source, IActionSink &sink)
    {
        bool ok1 = source.remove_sink(sink);
        bool ok2 = sink.add_source(source);

        if (ok1) ok1 = ok1 && source.on_disconnect(sink);
        if (ok2) ok2 = ok2 && sink.on_disconnect(source);
        return ok1 && ok2;
    }

}}


