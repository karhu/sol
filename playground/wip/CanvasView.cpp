#include "CanvasView.hpp"

#include "Canvas.hpp"

#include "miro/action/connect.hpp"
#include "miro/action/ActionDefinitions.hpp"

CanvasView::CanvasView()
{
}

CanvasView::~CanvasView()
{

}

bool CanvasView::set_canvas(miro::Canvas &canvas)
{
    if (m_canvas) {
        miro::action::disconnect(get_action_source(), m_canvas->sink_unconfirmed());
    }
    m_canvas = &canvas;
    miro::action::connect(get_action_source(), m_canvas->sink_unconfirmed());

    return true;
}

void CanvasView::set_transform(Transform2f transform)
{
    m_transform = transform;
}

miro::action::IActionSource &CanvasView::get_action_source()
{
    return m_writer;
}

void CanvasView::handle_cursor_event(const sol::CursorEvent &event)
{
    using Action = sol::CursorEvent::Action;
    using namespace miro::action;

    if (!m_canvas) return;

    uint8_t type = 0;

    if (event.action == Action::Down) {
        m_down = true;
        type = 2;
    } else if (event.action == Action::Up ) {
        m_down = false;
        type = 1;
    } else if (event.action == Action::Move) {
        if (!m_down) return;
        type = 0;
    }

    bool written;
    written = miro::action::write_stroke_action(
                m_writer.buffer(),HeaderMeta(0),
                event.position,
                event.pressure,
                1, type);

    if (!written) {
        std::cout << "write error" << std::endl;
        // TODO error
    }
    m_writer.send_and_reset();
}

