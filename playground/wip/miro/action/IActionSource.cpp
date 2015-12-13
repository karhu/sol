#include "IActionSource.hpp"

#include "util.hpp"

#include "IActionSink.hpp"

namespace miro { namespace action {

    void IActionSource::send(ActionRange action_range)
    {
        for (auto& s : m_sinks) {
            s->receive(action_range);
        }
    }

    bool IActionSource::on_connect(IActionSink &sink)
    {
        UNUSED(sink);
        return true;
    }

    bool IActionSource::on_disconnect(IActionSink &sink)
    {
        UNUSED(sink);
        return true;
    }

    bool IActionSource::add_sink(IActionSink &sink)
    {
        m_sinks.push_back(&sink);
        return true;
    }

    bool IActionSource::remove_sink(IActionSink &sink)
    {
        return sol::bag_remove_first(m_sinks,&sink);
    }

}}

