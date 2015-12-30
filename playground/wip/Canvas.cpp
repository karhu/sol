#include "Canvas.hpp"

#include <iostream>

//#include <GL/gl3w.h>
#include <GL/gl.h>

#include "nanovg.h"
#include "nanovg_gl_utils.h"

#include "Transform2.hpp"

#include "miro/action/ActionDefinitions.hpp"
#include "miro/action/BufferingActionSink.hpp"

using T2 = Transform2;

namespace miro {

Canvas::Canvas(sol::RenderContext& rctx, uint32_t width, uint32_t height)
    : m_render_context(rctx)
{
    m_sink_confirmed.reset(new action::BufferingActionSink()); // TODO change this later
    m_sink_unconfirmed.reset(new action::BufferingActionSink());
    m_render_target = m_render_context.create_render_target(width,height);

    m_render_context.begin_frame(m_render_target);
    m_render_context.bind(m_render_target);
    glClearColor(1.0f,1.0f,1.0f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    m_render_context.end_frame();
}

action::IActionSink &Canvas::sink_unconfirmed()
{
    return *m_sink_unconfirmed;
}

action::IActionSink &Canvas::sink_confirmed()
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

    //m_transform_winr_canvasa = t_winr_canvasa;

    m_render_context.begin_frame(m_render_target);
    m_render_context.bind(m_render_target);
    nvgResetTransform(vg);
    nvgReset(vg);

    using AT = miro::action::ActionType;

    // render unconfirmed
#if 0
    nvgFillColor(vg, nvgRGBA(255,0,0,64));
    m_sink_unconfirmed->handle_actions([this,vg](action::ActionRange range){
        for (uint16_t i=0; i < range.count(); i++) {
            auto ar = range.get(i);
            switch (ar.header().meta.type) {
                case AT::Stroke:
                {
                    auto a = ar.data<action::StrokeActionRef>();
                    auto p = a.position();
                    p = transform_point(p,m_transform_winr_canvasa);

                    nvgBeginPath(vg);
                    nvgCircle(vg, p.x, p.y, 3);
                    nvgFill(vg);
                    break;
                }
                case AT::Viewport:
                {
                    auto a = ar.data<action::ViewportActionRef>();
                    auto& t = a.transform();
                    m_transform_winr_canvasa = t;
                }
                default:
                    std::cout << "unhandled action type" << std::endl;
            }
        }
        return true;
    });
#endif
    // render confirmed
    nvgFillColor(vg, nvgRGBA(0,0,255,64));
    m_sink_confirmed->handle_actions([this,vg](action::ActionRange range){
        for (uint16_t i=0; i < range.count(); i++) {
            auto ar = range.get(i);
            switch (ar.header().meta.type) {
                case miro::action::ActionType::Stroke:
                {
                    auto a = ar.data<action::StrokeActionRef>();
                    auto uc = get_user_context(a.header().user);
                    if (uc != nullptr) {
                        auto p = a.position();
                        p = transform_point(p,uc->m_transform);

                        std::cout << (int)a.header().user << std::endl;

                        nvgBeginPath(vg);
                        nvgCircle(vg, p.x, p.y, 3);
                        nvgFill(vg);
                    }
                    break;
                }
                case AT::Viewport:
                {
                    auto a = ar.data<action::ViewportActionRef>();
                    auto uc = get_user_context(a.header().user);
                    if (uc != nullptr) {
                        uc->m_transform = a.transform();
                        std::cout << "vp1: " << (int)a.header().user << std::endl;
                    } else {
                        std::cout << "vp2: " << (int)a.header().user << std::endl;
                    }
                    break;
                }
                case AT::User:
                {
                    auto a = ar.data<action::UserActionRef>();
                    std::cout << "user: \n";
                    std::cout << "  alias: " << std::string(a.alias().ptr(),a.alias().size()) << "\n";
                    std::cout << "  idx:   " << a.idx() << "\n";
                    std::cout << "  kind:  " << (int)a.kind() << "\n";
                    std::cout << "  flags: " << (int)a.flags() << "\n";
                    std::cout << std::endl;
                    handle_user_action(a);
                    break;
                }
                default:
                    std::cout << "unhandled action type" << std::endl;
            }
        }
        return true;
    });

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

vec2f Canvas::dimensions() const
{
    return (vec2f)m_render_target.dimensions();
}

UserContext* Canvas::get_user_context(uint16_t idx)
{
    if (idx == 0) return nullptr;
    if (idx >= m_user_contexts.size()) return nullptr;
    return &m_user_contexts[idx];
}

void Canvas::handle_user_action(action::UserActionRef &action)
{
    auto idx = action.idx();
    if (idx >= m_user_contexts.size()) m_user_contexts.resize(idx+1);
    m_user_contexts[idx] = UserContext{
        Transform2(),
        std::string(action.alias().ptr(),action.alias().size()),
        idx
    };

    if ((int)action.flags() & (int)action::UserActionRef::Flag::Local) {
        m_local_user_idx = idx;
    }
}

}
