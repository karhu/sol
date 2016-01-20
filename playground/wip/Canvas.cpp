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
    : m_render_context(rctx),
      m_user_contexts(1)
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

    //m_transform_winr_canvasa = t_winr_canvasa;

    m_render_context.begin_frame(m_render_target);
    m_render_context.bind(m_render_target);
    nvgResetTransform(vg);
    nvgReset(vg);

    using AT = miro::action::ActionType;

    // render unconfirmed

    // render confirmed

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
                        auto pressure = a.pressure();

                        auto idx = a.header().user;
                        p = transform_point(p,uc->stroke_transform(*this));
                        //std::cout << "p " << p.x << ", " << p.y << std::endl;

                        auto c = uc->m_color;
                        nvgFillColor(vg, nvgRGBA(c.r*255,c.g*255,c.b*255,c.a*255*pressure));
                        nvgBeginPath(vg);
                        //nvgCircle(vg, p.x, p.y, 3);
                        nvgCircle(vg,p.x,p.y,10.0f*pressure);
                        nvgPathWinding(vg, NVG_SOLID);

                        nvgFill(vg);
                    }
                    break;
                }
                case AT::Viewport:
                {
                    auto a = ar.data<action::ViewportActionRef>();
                    auto uc = get_user_context(a.header().user);
                    if (uc != nullptr) {
                        uc->update_transform(*this,a);
                        //uc->m_stroke_transform = a.transform();
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
    auto vg = m_render_context.impl();
    const vec2f fb_dim = (vec2f)m_render_target.dimensions();

    auto uc = get_user_context(m_local_user_idx);
    if (!uc) {
        std::cout << "no user context" << std::endl;
        return;
    }

    // draw a rectangle in canvas space
    nvgResetTransform(vg);
    auto f = uc->canvas_transform(*this).data();
    nvgTransform(vg, f[0],f[1],f[2],f[3],f[4],f[5]);
    nvgBeginPath(vg);
    nvgRect(vg, 0, 0, 1, 1);

    // fill it with the canvas texture
    nvgResetTransform(vg);
    auto t2 = T2::Scale(vec2f{1,-1}/fb_dim) *
        T2::Translation(vec2f{0,1}) *
        uc->canvas_transform(*this);
    f = t2.data();
    nvgTransform(vg, f[0],f[1],f[2],f[3],f[4],f[5]);
    nvgFillPaint(vg, m_render_context.nvg_paint(m_render_target));
    nvgFill(vg);
}

vec2f Canvas::dimensions() const
{
    return (vec2f)m_render_target.dimensions();
}

UserContext* Canvas::get_user_context(uint16_t idx)
{
    //if (idx == 0) return nullptr;
    if (idx >= m_user_contexts.size()) return nullptr;
    return &m_user_contexts[idx];
}

UserContext *Canvas::get_local_user_context()
{
    return get_user_context(m_local_user_idx);
}

void Canvas::handle_user_action(action::UserActionRef &action)
{
    auto idx = action.idx();
    if (idx >= m_user_contexts.size()) m_user_contexts.resize(idx+1);
    m_user_contexts[idx] = UserContext{};
    m_user_contexts[idx].m_alias = std::string(action.alias().ptr(),action.alias().size());
    m_user_contexts[idx].m_id = idx;

    if ((int)action.flags() & (int)action::UserActionRef::Flag::Local) {
        m_local_user_idx = idx;
    }
}

void UserContext::update_transform(const Canvas& canvas, action::ViewportActionRef &action)
{
    m_canvas_position = action.position();
    m_canvas_rotation = action.rotation();
    m_canvas_scale = action.scale();
    m_view_dimensions = action.viewport_dim();
    m_canvas_transform_dirty = true;
    m_stroke_transform_dirty = true;
}

const Transform2 &UserContext::stroke_transform(const Canvas& canvas)
{
    if (m_stroke_transform_dirty) {
        m_stroke_transform =
            T2::Translation(vec2f{-0.5f,-0.5f}*canvas.dimensions()) *
            T2::Scale(m_canvas_scale) *
            T2::Rotation(m_canvas_rotation) *
            T2::Scale(vec2f{1,1}/(vec2f)m_view_dimensions) *
            T2::Translation(m_canvas_position);
        m_stroke_transform = m_stroke_transform.inverse();
        m_stroke_transform_dirty = false;
    }
    return m_stroke_transform;
}

const Transform2 &UserContext::canvas_transform(const Canvas &canvas)
{
    if (m_canvas_transform_dirty) {
        m_canvas_transform =
            T2::Translation({-0.5f,-0.5f}) *
            T2::Scale(canvas.dimensions()*m_canvas_scale) *
            T2::Rotation(m_canvas_rotation) *
            T2::Translation(m_canvas_position * m_view_dimensions) *
            T2::Scale(vec2f{1,-1}) *
            T2::Translation(vec2f{0,1}*m_view_dimensions);
        m_canvas_transform_dirty = false;
    }
    return m_canvas_transform;
}


}
