#pragma once

#include "vec2.hpp"
#include "Transform2.hpp"
#include "context.hpp"

#include "miro/action/ActionWriter.hpp"

namespace miro {
    class Canvas;
}

class CanvasView
{
public:
    CanvasView(sol::Context& context);
    ~CanvasView();
public:
    bool set_canvas(miro::Canvas& canvas);
public:
    miro::action::IActionSource& get_action_source();
public:
    void handle_cursor_event(const sol::CursorEvent& event);
    void handle_window_event(const sol::WindowEvent& event);
protected:
    void assert_view_clean();
    void assert_write_ok(bool written);
private:
    miro::action::ActionWriter m_writer;
    Transform2 m_transform;
    miro::Canvas* m_canvas = nullptr;
    bool m_down = false;

    bool m_view_dirty = true;
    sol::Context& m_context;

    vec2f m_canvas_position = {0.5,0.5};
    float m_canvas_rotation = 30.0f;
    float m_canvas_scale = 1.0f;
};

