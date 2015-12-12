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

void miro::IActionSource::send(actions::ActionRange action_range)
{
    for (auto& s : m_sinks) {
        s->receive(action_range);
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

void miro::IActionSink::receive(actions::ActionRange action_range)
{
   on_receive(action_range);
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

void miro::IActionSink::on_receive(actions::ActionRange action_range)
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

miro::BufferingActionSink::BufferingActionSink()
    : m_current_buffer(new actions::ActionBuffer())
{
}


void miro::BufferingActionSink::on_receive(miro::actions::ActionRange range)
{
    auto count = range.count();
    while (true) {
        range = m_current_buffer->copy_action(range);
        if (range.count() == 0 ) break;
        end_current_write_buffer();
    }
    m_count += count;
}

void miro::BufferingActionSink::end_current_write_buffer()
{
    m_full_buffers.push_back(std::move(m_current_buffer));
    if (m_available_buffers.empty())
        m_current_buffer.reset(new actions::ActionBuffer());
    else {
        m_current_buffer = std::move(m_available_buffers.back());
        m_available_buffers.pop_back();
    }
}

void miro::ConcurrentActionForwarder::poll()
{
    m_current_buffer->count();
    handle_actions([this](actions::ActionRange range){
       if (range.count() > 0)
           send(range);
       return true;
    });
}

void miro::ActionForwarder::on_receive(miro::Action action)
{
    send(action);
}

const miro::actions::ActionHeader &miro::actions::ActionRef::header() const
{
    return m_buffer->m_headers[m_index];
}

miro::actions::MemoryRange miro::actions::ActionRef::data_memory()
{
    return m_buffer->get_memory(header().memory);
}


void miro::ConcurrentBufferingActionSink::set_notify_callback(sol::delegate<void ()> cb)
{
    m_notify_cb = cb;
}

void miro::ConcurrentBufferingActionSink::on_receive(miro::actions::ActionRange range)
{
    {
    std::lock_guard<std::mutex> lock(m_mutex);
    BufferingActionSink::on_receive(range);
    }
    if (m_notify_cb) m_notify_cb();
}
