#pragma once

#include "vec2.hpp"
#include "Transform2.hpp"
#include "context.hpp"
#include "color.hpp"

#include "miro/action/ActionWriter.hpp"

namespace miro {
    class Canvas;
}

namespace sol {
    class RenderContext;
}

class ColorWheel {
public:
    void render(sol::Context &ctx, sol::RenderContext& rctx);
    bool handle_cursor(const sol::CursorEvent& event);
public:
    void activate(vec2f center, sol::color::RGBA color);
    void deactivate();
    bool active();
public:
    void set_color_change_cb(auto&& cb);
private:
    void update_color(sol::color::HSVA color, bool notify = false);
private:
    vec2f m_center = {200,200};
    sol::color::HSVA m_color = {0,1,1,1};
    float m_radius_outer = 76.0f;
    float m_radius_inner = 58.0f;
    float m_radius_triangle = 50.0f;
    bool m_active = false;
    std::function<void(sol::color::RGBA)> m_color_change_cb;
private:
    enum Interaction {
        None = 0,
        Wheel = 1,
        Triangle = 2,
    } m_interaction = Interaction::None;
};

void ColorWheel::set_color_change_cb(auto&& cb)
{
    m_color_change_cb = cb;
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
public:
    void dirty_view();
    void assert_view_clean(bool send = false);
protected:
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
    bool m_mode_color_wheel = true;
    vec2f m_transform_last;

    sol::CursorEvent m_last_cursor_event;

private:
    struct TransformWidget {
        vec2f center;
        bool active = false;

        struct Rotation {
            bool active = false;
            bool hovering = false;
            float start_value;
            float canvas_start_rotation;
            vec2f canvas_start_position;
            float radius_inner = 55;
            float radius_outer = 73;
        } rotation;

    } m_transform_widget;

    ColorWheel m_color_wheel_widget;
};

