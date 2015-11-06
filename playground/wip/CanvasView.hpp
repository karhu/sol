#pragma once

#include "vec2.hpp"
#include "Action.hpp"
#include "context.hpp"

namespace miro {
    class Canvas;
}

class CanvasView : public miro::IActionSource
{
public:
    CanvasView();
    ~CanvasView();
public:
    bool set_canvas(miro::Canvas& canvas);
    void set_transform(Transform2f transform);
public:
    void handle_cursor_event(const sol::CursorEvent &event);
private:
    Transform2f m_transform;
    miro::Canvas* m_canvas = nullptr;
    bool m_down = false;
};

