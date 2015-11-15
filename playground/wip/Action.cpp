#include "Action.hpp"

#include <algorithm>
#include "util.hpp"

bool miro::connect(miro::IActionSource &source, miro::IActionSink &sink)
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


bool miro::disconnect(miro::IActionSource &source, miro::IActionSink &sink)
{
    bool ok1 = source.remove_sink(sink);
    bool ok2 = sink.add_source(source);

    if (ok1) ok1 = ok1 && source.on_disconnect(sink);
    if (ok2) ok2 = ok2 && sink.on_disconnect(source);
    return ok1 && ok2;
}


void miro::IActionSource::send(const miro::Action &action)
{
    for (auto& s : m_sinks) {
        s->receive(action);
    }
}

bool miro::IActionSource::on_connect(miro::IActionSink &sink)
{
    return true;
}

bool miro::IActionSource::on_disconnect(miro::IActionSink &sink)
{
    return true;
}

bool miro::IActionSource::add_sink(miro::IActionSink &sink)
{
    m_sinks.push_back(&sink);
    return true;
}

bool miro::IActionSource::remove_sink(miro::IActionSink &sink)
{
    return sol::bag_remove_first(m_sinks,&sink);
}


void miro::IActionSink::receive(miro::Action action)
{
   on_receive(action);
}

bool miro::IActionSink::on_connect(miro::IActionSource &source)
{
    return true;
}

bool miro::IActionSink::on_disconnect(miro::IActionSource &source)
{
    return true;
}

void miro::IActionSink::on_receive(miro::Action action)
{
    // noop
}

bool miro::IActionSink::add_source(miro::IActionSource &source)
{
    m_sources.push_back(&source);
    return true;
}

bool miro::IActionSink::remove_source(miro::IActionSource &source)
{
    return sol::bag_remove_first(m_sources,&source);
}


uint32_t miro::BufferingActionSink::count() const
{
    return m_buffer.size();
}

miro::Action miro::BufferingActionSink::peak_front() const
{
    return m_buffer.front();
}

miro::Action miro::BufferingActionSink::pop_front()
{
    Action tmp = m_buffer.front();
    m_buffer.pop_front();
    return tmp;
}

void miro::BufferingActionSink::on_receive(miro::Action action)
{
    m_buffer.push_back(action);
}


uint32_t miro::ConcurrentActionBuffer::poll()
{
    std::deque<Action> tmp;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::swap(tmp,m_buffer);
    }
    for (auto& a : tmp) {
        send(a);
    }
    return tmp.size();
}

void miro::ConcurrentActionBuffer::on_receive(miro::Action action)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_buffer.push_back(action);
}


void miro::ActionForwarder::on_receive(miro::Action action)
{
    send(action);
}
