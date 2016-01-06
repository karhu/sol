#include "CanvasView.hpp"

#include "Canvas.hpp"

#include "miro/action/connect.hpp"
#include "miro/action/ActionDefinitions.hpp"

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

void CanvasView::handle_cursor_event(const sol::CursorEvent &event)
{
    using Action = sol::CursorEvent::Action;
    using namespace miro::action;

    std::cout << "ce: " << event.position.x << " / " << event.position.y << std::endl;

    if (!m_canvas) return;

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
        if (event.is_press())
            m_transform_mode = true;
        else
            m_transform_mode = false;
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
        const vec2f canvas_dim = m_canvas->dimensions();

        // set up the necessary transforms
        vec2f scaled_dim_px = canvas_dim * m_canvas_scale;
        vec2f offset_px = m_canvas_position * win_dim;

        // transform from relative canvas coordinates to absolute windows coordinates
//        const T2 t_canvas_view = T2::Scale(scaled_dim_px)
//              * T2::Translation(scaled_dim_px*-0.5f)
//              * T2::Rotation(-m_canvas_rotation) // why do we have to invert the rotation here?
//              * T2::Translation(offset_px);

        //const T2 t_viewa_canvasr = t_canvas_view.inverse();
        const T2 t_viewr_canvasa =
                  T2::Translation(-m_canvas_position)
                * T2::Scale(win_dim)
                * T2::Rotation(m_canvas_rotation)
                * T2::Scale(vec2f{1,1}/m_canvas_scale)
                * T2::Translation(canvas_dim*0.5f);
                //* T2::Translation(scaled_dim_px*0.5f)
                //* T2::Scale(vec2f{1,1}/m_canvas_scale);
//      const T2 t_viewr_canvasa = T2::Scale(win_dim) * t_viewa_canvasr * T2::Scale(canvas_dim);

        auto transform = t_viewr_canvasa;

        // notify
        assert_write_ok(write_viewport_action(
            m_writer.buffer(),HeaderMeta(0),
            transform,
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

