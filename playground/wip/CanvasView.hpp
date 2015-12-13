#pragma once

#include "vec2.hpp"
#include "context.hpp"

#include "miro/action/ActionWriter.hpp"

namespace miro {
    class Canvas;
}

class CanvasView
{
public:
    CanvasView();
    ~CanvasView();
public:
    bool set_canvas(miro::Canvas& canvas);
    void set_transform(Transform2f transform);
public:
    miro::action::IActionSource& get_action_source();
public:
    void handle_cursor_event(const sol::CursorEvent &event);
private:
    miro::action::ActionWriter m_writer;
    Transform2f m_transform;
    miro::Canvas* m_canvas = nullptr;
    bool m_down = false;
};

