#include "IActionSink.hpp"

#include "util.hpp"

namespace miro { namespace action {

    void IActionSink::receive(ActionRange action_range)
    {
       on_receive(action_range);
    }

    bool IActionSink::on_connect(IActionSource &source)
    {
        return true;
    }

    bool IActionSink::on_disconnect(IActionSource &source)
    {
        return true;
    }

    void IActionSink::on_receive(ActionRange action_range)
    {
       // noop
    }

    bool IActionSink::add_source(IActionSource &source)
    {
        m_sources.push_back(&source);
        return true;
    }

    bool IActionSink::remove_source(IActionSource &source)
    {
        return sol::bag_remove_first(m_sources,&source);
    }

}}

