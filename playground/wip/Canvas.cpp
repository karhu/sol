#include "Canvas.hpp"

#include <iostream>

//#include <GL/gl3w.h>
#include <GL/gl.h>

#include "nanovg.h"
#include "nanovg_gl_utils.h"


#include "Transform2.hpp"

using T2 = Transform2;
static float rot = 0.0f;

namespace miro {

class SinkConfirmed : public BufferingActionSink
{
public:
    SinkConfirmed(Canvas& c) : m_canvas(c) {}
    ~SinkConfirmed() {}
private:
    Canvas& m_canvas;
};

class SinkUnconfirmed : public BufferingActionSink
{
public:
    SinkUnconfirmed(Canvas& c) : m_canvas(c) {}
    ~SinkUnconfirmed() {}
private:
    Canvas& m_canvas;
};

Canvas::Canvas(sol::RenderContext& rctx, uint32_t width, uint32_t height)
    : m_render_context(rctx)
{
    m_sink_confirmed.reset(new SinkConfirmed(*this));
    m_sink_unconfirmed.reset(new SinkUnconfirmed(*this));
    m_render_target = m_render_context.create_render_target(width,height);

    m_render_context.begin_frame(m_render_target);
    m_render_context.bind(m_render_target);
    glClearColor(1.0f,1.0f,1.0f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    m_render_context.end_frame();
}

IActionSink &Canvas::sink_unconfirmed()
{
    return *m_sink_unconfirmed;
}

IActionSink &Canvas::sink_confirmed()
{
    return *m_sink_confirmed;
}

void Canvas::update(sol::Context& ctx)
{
    auto& windows = ctx.windows();
    auto vg = m_render_context.impl();
    const vec2f win_dim = windows.get_render_target_dimensions(windows.get_main_window());
    const vec2f fb_dim = (vec2f)m_render_target.dimensions();

    // set up the necessary transforms
    vec2f scaled_dim_px = fb_dim * m_scale;
    vec2f offset_px = m_position * win_dim;

    // transform from relative canvas coordinates to absolute windows coordinates
    const T2 t_canvas_win = T2::Scale(scaled_dim_px)
            * T2::Translation(scaled_dim_px*-0.5f)
            * T2::Rotation(-m_rotation) // why do we have to invert the rotation here?
            * T2::Translation(offset_px);

    const T2 t_wina_canvasr = t_canvas_win.inverse();
    const T2 t_winr_canvasa = T2::Scale(win_dim) * t_wina_canvasr * T2::Scale(fb_dim);

    m_render_context.begin_frame(m_render_target);
    m_render_context.bind(m_render_target);

    nvgResetTransform(vg);
    nvgReset(vg);
    nvgFillColor(vg, nvgRGBA(255,0,0,128));

    auto count = m_sink_unconfirmed->count();
    for (uint32_t i=0; i<count; i++)
    {
        auto a = m_sink_unconfirmed->pop_front();
        auto p = a.data.stroke.position;
        p = transform_point(p,t_winr_canvasa);

        nvgBeginPath(vg);
        nvgCircle(vg, p.x, p.y, 3);
        nvgFill(vg);
    }

    m_render_context.end_frame();
}

void Canvas::render(sol::Context &ctx)
{
    auto& windows = ctx.windows();
    auto vg = m_render_context.impl();

    const vec2f win_dim = windows.get_render_target_dimensions(windows.get_main_window());
    const vec2f fb_dim = (vec2f)m_render_target.dimensions();

    //m_rotation += 0.01f;

    vec2f scaled_dim_px = fb_dim * m_scale;
    vec2f offset_px = m_position * win_dim;

    // transform from relative canvas coordinates to absolute windows coordinates
    const T2 t_canvas_win = T2::Scale(scaled_dim_px)
            * T2::Translation(scaled_dim_px*-0.5f)
            * T2::Rotation(m_rotation) // why do we have to invert the rotation here?
            * T2::Translation(offset_px);

    m_render_context.begin_frame(ctx.windows().get_main_window());
    m_render_context.bind(sol::RenderTarget::Default);

    glClearColor(0.75f,0.75f,0.75f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

    nvgResetTransform(vg);

    auto f = t_canvas_win.data();
    nvgTransform(vg, f[0],f[1],f[2],f[3],f[4],f[5]);
    nvgBeginPath(vg);
    nvgRect(vg, 0, 0, 1, 1);
    nvgResetTransform(vg);

    auto t = Transform2::TSR({0,0},vec2f{1,1}/fb_dim,0.0f) * t_canvas_win ;
    f = t.data();
    nvgTransform(vg, f[0],f[1],f[2],f[3],f[4],f[5]);
    nvgFillPaint(vg, m_render_context.nvg_paint(m_render_target));

    nvgFill(vg);
    m_render_context.end_frame();

}

}
