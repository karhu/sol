#include "CanvasView.hpp"

#include "Canvas.hpp"

#include "render_context.hpp"

#include <GL/gl.h>
#include "nanovg.h"
#include "nanovg_gl_utils.h"

#include "miro/action/connect.hpp"
#include "miro/action/ActionDefinitions.hpp"

#include "math.hpp"

using T2 = Transform2;

CanvasView::CanvasView(sol::Context& context)
    : m_context(context)
{
    m_color_wheel_widget.set_color_change_cb([this](sol::color::RGBA color)
    {
        using Action = sol::CursorEvent::Action;
        using namespace miro::action;

        assert_write_ok(write_color_action(
            m_writer.buffer(),HeaderMeta(0),
            color
        ));
        m_writer.send_and_reset();
    });
}

CanvasView::~CanvasView()
{}

bool CanvasView::set_canvas(miro::Canvas &canvas)
{
    if (m_canvas) {
        miro::action::disconnect(get_action_source(), m_canvas->sink_unconfirmed());
    }
    m_canvas = &canvas;
    miro::action::connect(get_action_source(), m_canvas->sink_unconfirmed());

    dirty_view();
    assert_view_clean(true);

    return true;
}

miro::action::IActionSource &CanvasView::get_action_source()
{
    return m_writer;
}

void CanvasView::render(sol::Context &ctx, sol::RenderContext& rctx)
{
    auto vg = rctx.impl();
    m_canvas->render(ctx);

    auto wh = ctx.windows().get_main_window();
    auto wdim = ctx.windows().get_window_dimensions(wh);

    nvgResetTransform(vg);
    nvgTranslate(vg,0,wdim.y);
    nvgScale(vg,1,-1);


    auto uc = m_canvas->get_local_user_context();

    if (m_transform_mode) {
        nvgSave(vg);
        auto& tw = m_transform_widget;
        T2 t =
            T2::Scale(vec2f{1,-1}) *
            T2::Translation(vec2f{0,1}*uc->m_view_dimensions);
        auto p = transform_point(tw.center,t);
        // draw a rectangle in canvas space
        nvgResetTransform(vg);
        //auto f = uc->canvas_transform(*this).data();
        //nvgTransform(vg, f[0],f[1],f[2],f[3],f[4],f[5]);
        nvgBeginPath(vg);
        nvgCircle(vg,p.x,p.y,tw.rotation.radius_inner);
        nvgPathWinding(vg, NVG_CCW);
        nvgCircle(vg,p.x,p.y,tw.rotation.radius_outer);
        nvgPathWinding(vg, NVG_CW);
        uint8_t alpha = tw.rotation.active || tw.rotation.hovering  ? 192 : 128;
        nvgFillColor(vg,nvgRGBA(255,0,0,alpha));
        nvgFill(vg);
        nvgRestore(vg);
    }

    if (m_color_wheel_widget.active()) {
        m_color_wheel_widget.render(ctx,rctx);
    }
}

void CanvasView::handle_cursor_event(const sol::CursorEvent &event)
{
    using Action = sol::CursorEvent::Action;
    using namespace miro::action;

    //std::cout << "ce: " << event.position.x << " / " << event.position.y << std::endl;

    if (!m_canvas) return;
    m_last_cursor_event = event;

    auto uc = m_canvas->get_local_user_context();

    if (m_transform_mode)
    {
        auto& tw = m_transform_widget;

        // cursor position relative to the widget center
        auto d = event.position * uc->m_view_dimensions;
        d = d - tw.center;
        auto r2 = dot(d,d);
        // is the cursor over the rotation ring?
        tw.rotation.hovering = r2 < tw.rotation.radius_outer*tw.rotation.radius_outer &&
                               r2 > tw.rotation.radius_inner*tw.rotation.radius_inner;
        if (event.action == Action::Down)
        {
            m_down = true;
            m_transform_last = event.position;
            if (tw.rotation.hovering)
            {
                // begin rotation
                tw.rotation.active = true;
                float angle = sol::rad2deg(atan2f(d.y,d.x));
                tw.rotation.start_value = angle;
                tw.rotation.canvas_start_rotation = uc->m_canvas_rotation;
                tw.rotation.canvas_start_position = uc->m_canvas_position * uc->m_view_dimensions;
            }

        }
        else if (event.action == Action::Up )
        {
            m_down = false;
            m_transform_widget.rotation.active = false;
        }
        else if (event.action == Action::Move)
        {
            if (!m_down) return;
            auto& tw = m_transform_widget;
            if (tw.rotation.active)
            {
                auto p = event.position * uc->m_view_dimensions;
                auto d = normalized(p - tw.center);
                float angle = sol::rad2deg(atan2f(d.y,d.x));
                angle = angle - tw.rotation.start_value;
                m_canvas_rotation = tw.rotation.canvas_start_rotation + angle;
                auto r = tw.rotation.canvas_start_position - tw.center;
                r = transform_point(r,T2::Rotation(angle));
                r = r + tw.center;
                r = r / uc->m_view_dimensions;
                m_canvas_position = r;
            }
            else
            {
                auto delta = event.position - m_transform_last;
                m_transform_last = event.position;
                m_canvas_position = m_canvas_position + delta;
            }
            m_view_dirty = true;
            assert_view_clean(true);
        }
    } else if (m_color_wheel_widget.active())
    {
        // TODO this should only be a temporary workaround
        sol::CursorEvent copy = event;
        auto wh =  m_context.windows().get_main_window();
        copy.position = copy.position * m_context.windows().get_window_dimensions(wh);
        m_color_wheel_widget.handle_cursor(copy);
    } else {
        using Kind = StrokeActionRef::Kind;
        Kind kind = Kind::Update;
        if (event.action == Action::Down) {
            m_down = true;
            kind = Kind::Begin;
        } else if (event.action == Action::Up ) {
            m_down = false;
            kind = Kind::End;
        } else if (event.action == Action::Move) {
            if (!m_down) return;
            kind = Kind::Update;
        }

        assert_view_clean();

        assert_write_ok(write_stroke_action(
            m_writer.buffer(),HeaderMeta(0),
            event.position,
            event.pressure,
            1, kind
        ));
        m_writer.send_and_reset();
    }
}

void CanvasView::handle_window_event(const sol::WindowEvent &event)
{
    dirty_view();
    assert_view_clean();
}

void CanvasView::handle_keyboard_event(const sol::KeyboardEvent &event)
{
    auto uc = m_canvas->get_local_user_context();

    if (event.key == sol::KeyboardKey::Space) {
        if (event.is_press()) {
            if (!m_transform_widget.active) {
                m_transform_mode = true;
                m_transform_last = m_last_cursor_event.position;
                m_transform_widget.center = m_transform_last*uc->m_view_dimensions;
                m_transform_widget.active = true;
            }
        } else {
            m_transform_mode = false;
            m_transform_widget.active = false;
        }
    }
    if (event.key == sol::KeyboardKey::C)
    {
        if (event.is_press())
        {
            if (!m_color_wheel_widget.active())
            {
                m_transform_last = m_last_cursor_event.position;
                auto center = m_transform_last*uc->m_view_dimensions;
                m_color_wheel_widget.activate(center,m_canvas->get_local_user_context()->m_color);
            }
        }
        else
        {
            m_color_wheel_widget.deactivate();
        }
    }
}

void CanvasView::dirty_view()
{
    m_view_dirty = true;
}

void CanvasView::assert_view_clean(bool send)
{
    using T2 = Transform2;

    using namespace miro::action;
    if (m_view_dirty)
    {
        // update transform
        auto& windows = m_context.windows();
        const vec2f win_dim = windows.get_render_target_dimensions(windows.get_main_window());

        // notify
        assert_write_ok(write_viewport_action(
            m_writer.buffer(),HeaderMeta(0),
            m_canvas_position,
            m_canvas_rotation,
            m_canvas_scale,
            (vec2u16)win_dim
        ));

        m_view_dirty = false;
    }
    if (send) m_writer.send_and_reset();
}

void CanvasView::assert_write_ok(bool written)
{
    // TODO
    if (!written) {
        std::cout << "write error" << std::endl;
    }
}

void ColorWheel::render(sol::Context &ctx, sol::RenderContext& rctx)
{
    NVGcontext* vg = rctx.impl();
    auto& center = m_center;

    auto hsv = m_color;
    float hue = (float)hsv.h;

    nvgSave(vg);

    float aeps = 0.5f / m_radius_outer;	// half a pixel arc length in radians (2pi cancels out).

    // wheel
    for (int i = 0; i < 6; i++)
    {
        float a0 = (float)i / 6.0f * NVG_PI * 2.0f - aeps;
        float a1 = (float)(i+1.0f) / 6.0f * NVG_PI * 2.0f + aeps;
        nvgBeginPath(vg);
        nvgArc(vg, center.x, center.y, m_radius_inner, a0, a1, NVG_CW);
        nvgArc(vg, center.x, center.y, m_radius_outer, a1, a0, NVG_CCW);
        nvgClosePath(vg);
        auto rt = m_radius_inner+m_radius_outer;
        float ax = center.x + cosf(a0) * rt*0.5f;
        float ay = center.y + sinf(a0) * rt*0.5f;
        float bx = center.x + cosf(a1) * rt*0.5f;
        float by = center.y + sinf(a1) * rt*0.5f;
        NVGpaint paint = nvgLinearGradient(vg, ax,ay, bx,by, nvgHSLA(a0/(NVG_PI*2),1.0f,0.55f,255), nvgHSLA(a1/(NVG_PI*2),1.0f,0.55f,255));
        nvgFillPaint(vg, paint);
        nvgFill(vg);
    }

    // wheel outline
    nvgBeginPath(vg);
    nvgCircle(vg, center.x, center.y, m_radius_inner-0.5f);
    nvgCircle(vg, center.x, center.y, m_radius_outer+0.5f);
    nvgStrokeColor(vg, nvgRGBA(0,0,0,64));
    nvgStrokeWidth(vg, 1.0f);
    nvgStroke(vg);


    // center triangle
    nvgTranslate(vg, center.x,center.y);
    nvgRotate(vg, hue*NVG_PI*2);

    auto r = m_radius_triangle;
    float ax = cosf(120.0f/180.0f*NVG_PI) * r;
    float ay = sinf(120.0f/180.0f*NVG_PI) * r;
    float bx = cosf(-120.0f/180.0f*NVG_PI) * r;
    float by = sinf(-120.0f/180.0f*NVG_PI) * r;
    nvgBeginPath(vg);
    nvgMoveTo(vg, r,0);
    nvgLineTo(vg, ax,ay);
    nvgLineTo(vg, bx,by);
    nvgClosePath(vg);
    auto paint = nvgLinearGradient(vg, r,0, ax,ay, nvgHSLA(hue,1.0f,0.5f,255), nvgRGBA(255,255,255,255));
    nvgFillPaint(vg, paint);
    nvgFill(vg);
    paint = nvgLinearGradient(vg, (r+ax)*0.5f,(0+ay)*0.5f, bx,by, nvgRGBA(0,0,0,0), nvgRGBA(0,0,0,255));
    nvgFillPaint(vg, paint);
    nvgFill(vg);
    nvgStrokeColor(vg, nvgRGBA(0,0,0,64));
    nvgStroke(vg);

    // select circle on triangle
    auto rot = Transform2::Rotation(120);
    auto pt_c = vec2f(1,0) * m_radius_triangle;
    auto pt_w = transform_point(pt_c,rot);
    auto pt_b = transform_point(pt_w,rot);
    float u = 1.0f - m_color.s;
    float v = 1.0f - m_color.v;
    float w = 1.0f - u - v;
    auto cp = u*pt_w + v*pt_b + w*pt_c;

    ax = cp.x;
    ay = cp.y;
    nvgStrokeWidth(vg, 2.0f);
    nvgBeginPath(vg);
    nvgCircle(vg, ax,ay,5);
    nvgStrokeColor(vg, nvgRGBA(255,255,255,192));
    nvgStroke(vg);

    paint = nvgRadialGradient(vg, ax,ay, 7,9, nvgRGBA(0,0,0,64), nvgRGBA(0,0,0,0));
    nvgBeginPath(vg);
    nvgRect(vg, ax-20,ay-20,40,40);
    nvgCircle(vg, ax,ay,7);
    nvgPathWinding(vg, NVG_HOLE);
    nvgFillPaint(vg, paint);
    nvgFill(vg);

    nvgRestore(vg);
}

bool ColorWheel::handle_cursor(const sol::CursorEvent &event)
{
    using CA = sol::CursorEvent::Action;

    auto d = event.position - m_center;
    auto r2 = dot(d,d);

    float angle = sol::rad2deg(atan2f(-d.y,-d.x)) ;
    auto color = m_color;
    float hue = (angle+180.0f) / 360.0f;

    //auto tst = vec2f(cos(sol::deg2rad(color.h*360)),sin(sol::deg2rad(color.h*360)));
    //std::cout << tst.x << " / " << tst.y << std::endl;

    auto rot = Transform2::Rotation(120);
    auto pt_color = vec2f(cos(sol::deg2rad(color.h*360)),sin(sol::deg2rad(color.h*360))) * m_radius_triangle;
    auto pt_white = transform_point(pt_color,rot);
    auto pt_black = transform_point(pt_white,rot);
    auto bc = sol::barycentric_coordinates(pt_color,pt_white,pt_black,d);
    auto bcw = 1.0f - bc.x - bc.y;
    bool inside_triangle = bc.x >= 0.0f &&
                           bc.y >= 0.0f &&
                           bcw  >= 0.0f;

    float sat = 1.0f - sol::clamp(bc.x,0.0f,1.0f);
    float val = 1.0f - sol::clamp(bc.y,0.0f,1.0f);

    std::cout << pt_white.x << " / " << pt_white.y << std::endl;

    switch (event.action)
    {
    case CA::Down: {
        auto ro2 = m_radius_outer*m_radius_outer;
        auto ri2 = m_radius_inner*m_radius_inner;
        auto rt2 = m_radius_triangle*m_radius_triangle;
        if (r2 <= ro2 && r2 >= ri2) {
            m_interaction = Interaction::Wheel;
            color.h = hue;
            update_color(color);
        }
        if (r2 <= rt2) {
            if (inside_triangle) {
                m_interaction = Interaction::Triangle;
                color.s = sat;
                color.v = val;
                update_color(color, true);
            }
        }
        break;
    }
    case CA::Move:
        if (m_interaction == Interaction::Wheel) {
            color.h = hue;
            update_color(color);
        }
        if (m_interaction == Interaction::Triangle) {
            color.s = sat;
            color.v = val;
            update_color(color);

            std::cout << color.h << "/" << color.s << "/" << color.v << std::endl;
        }
        break;
    case CA::Up:
        if (m_interaction == Interaction::Wheel) {
            color.h = hue;
            update_color(color, true);
        }
        if (m_interaction == Interaction::Triangle) {
            color.s = sat;
            color.v = val;
            update_color(color, true);
        }
        m_interaction = Interaction::None;
        break;
    }
}

void ColorWheel::activate(vec2f center, sol::color::RGBA color)
{
    m_center = center;
    m_color = sol::color::rgba2hsva(color);
    m_active = true;
}

void ColorWheel::deactivate()
{
    m_active = false;
    if (m_interaction == Interaction::Wheel || m_interaction == Interaction::Triangle) {
        update_color(m_color, true);
    }
    m_interaction = Interaction::None;
}

bool ColorWheel::active()
{
    return m_active;
}

void ColorWheel::update_color(sol::color::HSVA color, bool notify)
{
    m_color = color;
    if (notify && m_color_change_cb) {
        m_color_change_cb(sol::color::hsva2rgba(m_color));
    }
    // TODO notify
}
