#include "CanvasView.hpp"

#include "Canvas.hpp"

#include "render_context.hpp"

#include <GL/gl.h>
#include "nanovg.h"
#include "nanovg_gl_utils.h"

#include "miro/action/connect.hpp"
#include "miro/action/ActionDefinitions.hpp"

using T2 = Transform2;

CanvasView::CanvasView(sol::Context& context)
    : m_context(context)
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

miro::action::IActionSource &CanvasView::get_action_source()
{
    return m_writer;
}

void CanvasView::render(sol::Context &ctx, sol::RenderContext& rctx)
{
    auto vg = rctx.impl();
    m_canvas->render(ctx);

    auto uc = m_canvas->get_local_user_context();

    if (m_transform_mode) {
        auto& tw = m_transform_widget;
        T2 t =
            T2::Scale(vec2f{1,-1}*uc->m_view_dimensions) *
            T2::Translation(vec2f{0,1}*uc->m_view_dimensions);
        auto p = transform_point(tw.center,t);
        // draw a rectangle in canvas space
        nvgResetTransform(vg);
        //auto f = uc->canvas_transform(*this).data();
        //nvgTransform(vg, f[0],f[1],f[2],f[3],f[4],f[5]);
        nvgBeginPath(vg);
        nvgCircle(vg,p.x,p.y,tw.radius_inner);
        nvgPathWinding(vg, NVG_CCW);
        nvgCircle(vg,p.x,p.y,tw.radius_outer);
        nvgPathWinding(vg, NVG_CW);
        nvgFillColor(vg,nvgRGBA(255,0,0,128));
        nvgFill(vg);
    }
}

void CanvasView::handle_cursor_event(const sol::CursorEvent &event)
{
    using Action = sol::CursorEvent::Action;
    using namespace miro::action;

    //std::cout << "ce: " << event.position.x << " / " << event.position.y << std::endl;

    if (!m_canvas) return;
    m_last_cursor_event = event;

    if (m_transform_mode) {
        if (event.action == Action::Down) {
            m_down = true;
            m_transform_last = event.position;
        } else if (event.action == Action::Up ) {
            m_down = false;
        } else if (event.action == Action::Move) {
            if (!m_down) return;
            auto delta = event.position - m_transform_last;
            m_transform_last = event.position;
            m_canvas_position = m_canvas_position + delta;
            m_view_dirty = true;
            assert_view_clean(true);
        }
    } else {
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

        assert_view_clean();

        assert_write_ok(write_stroke_action(
            m_writer.buffer(),HeaderMeta(0),
            event.position,
            event.pressure,
            1, type
        ));
        m_writer.send_and_reset();
    }
}

void CanvasView::handle_window_event(const sol::WindowEvent &event)
{
    dirty_view();
}

void CanvasView::handle_keyboard_event(const sol::KeyboardEvent &event)
{
    if (event.key == sol::KeyboardKey::Space) {
        if (event.is_press()) {
            if (!m_transform_widget.active) {
                m_transform_mode = true;
                m_transform_last = m_last_cursor_event.position;
                m_transform_widget.center = m_transform_last;
                m_transform_widget.active = true;
            }
        } else {
            m_transform_mode = false;
            m_transform_widget.active = false;
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

