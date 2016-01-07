#pragma once

#include "vec2.hpp"
#include "Transform2.hpp"
#include "context.hpp"

#include "miro/action/ActionWriter.hpp"

namespace miro {
    class Canvas;
}

namespace sol {
    class RenderContext;
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
    void render(sol::Context& ctx, sol::RenderContext &rctx);
public:
    void handle_cursor_event(const sol::CursorEvent& event);
    void handle_window_event(const sol::WindowEvent& event);
    void handle_keyboard_event(const sol::KeyboardEvent& event);
protected:
    void dirty_view();
    void assert_view_clean(bool send = false);
    void assert_write_ok(bool written);
private:
    miro::action::ActionWriter m_writer;
    //Transform2 m_transform;
    miro::Canvas* m_canvas = nullptr;
    bool m_down = false;

    bool m_view_dirty = true;
    sol::Context& m_context;

    vec2f m_canvas_position = {0.5,0.5};
    float m_canvas_rotation = 0.0f;
    float m_canvas_scale = 1.0f;

    bool m_transform_mode = false;
    vec2f m_transform_last;

    sol::CursorEvent m_last_cursor_event;

private:
    struct TransformWidget {
        vec2f center;
        bool active = false;

        bool  rotation = false;
        float radius_inner = 40;
        float radius_outer = 55;
        float rotation_start_value;
        vec2f rotation_start_center;
        float rotation_start_rot;

    } m_transform_widget;
};

