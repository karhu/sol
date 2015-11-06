#include "CanvasView.hpp"

#include "Canvas.hpp"

CanvasView::CanvasView()
{
}

CanvasView::~CanvasView()
{

}

bool CanvasView::set_canvas(miro::Canvas &canvas)
{
    if (m_canvas) {
        miro::disconnect(*this, m_canvas->sink_unconfirmed());
    }
    m_canvas = &canvas;

    miro::connect(*this, m_canvas->sink_unconfirmed());

    return true;
}

void CanvasView::set_transform(Transform2f transform)
{
    m_transform = transform;
}

void CanvasView::handle_cursor_event(const sol::CursorEvent &event)
{
    using Action = sol::CursorEvent::Action;

    if (!m_canvas) return;

    miro::Action a;
    a.timestamp = 0; // TODO

    a.data.stroke.button = 1; // TODO
    a.data.stroke.position = event.position;
    a.data.stroke.pressure = event.pressure;

    // TODO transform position

    if (event.action == Action::Down) {
        m_down = true;
        a.type = miro::ActionType::StrokeBegin;
    } else if (event.action == Action::Up ) {
        m_down = false;
        a.type = miro::ActionType::StrokeEnd;
    } else if (event.action == Action::Move) {
        if (!m_down) return;
        a.type = miro::ActionType::StrokeUpdate;
    }

    this->send(a);
}

