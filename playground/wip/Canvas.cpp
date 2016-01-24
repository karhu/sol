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

    // overlay rendertarget for temporary drawing
    m_render_target_tmp = m_render_context.create_render_target(width,height);
    m_render_context.begin_frame(m_render_target_tmp);
    m_render_context.bind(m_render_target_tmp);
    glClearColor(0.0f,0.0f,0.0f,0.0f);
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
    auto vg = m_render_context.impl();

    m_render_context.begin_frame(m_render_target);
    m_render_context.bind(m_render_target);
    nvgResetTransform(vg);
    nvgReset(vg);

    using AT = miro::action::ActionType;

    // handle unconfirmed actions
    m_sink_unconfirmed->handle_actions([this,vg](action::ActionRange range)
    {
        auto uc = get_local_user_context();
        for (uint16_t i=0; i < range.count(); i++)
        {
            auto ar = range.get(i);
            switch (ar.header().meta.type)
            {
                case miro::action::ActionType::Stroke:
                {
                    using Kind = action::StrokeActionRef::Kind;
                    auto a = ar.data<action::StrokeActionRef>();

                    auto idx = m_local_user_idx;
                    auto p = a.position();
                    p = transform_point(p,uc->stroke_transform(*this));
                    auto pressure = a.pressure();

                    if (uc != nullptr) {
                        // new stroke building interface
                        if (a.kind() == Kind::Update)
                        {
                            m_strokes.add_point(idx,StrokePoint{p,pressure});
                        }
                        else if (a.kind() == Kind::Begin)
                        {
                            m_strokes.begin_stroke(idx,false,StrokeProperties{uc->m_color});
                            m_strokes.add_point(idx,StrokePoint{p,pressure});
                        }
                        else if (a.kind() == Kind::End)
                        {
                            m_strokes.end_stroke(idx);
                        }
                    }
                    break;
                }
                case AT::Viewport:
                {
                    auto a = ar.data<action::ViewportActionRef>();
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
                case AT::Color:
                {
                    auto a = ar.data<action::ColorActionRef>();
                    if (uc != nullptr) {
                        uc->m_color = a.color();
                    }
                    break;
                }
                default:
                    std::cout << "unhandled action type" << std::endl;
            }
        }
        return true;
    });

    // handle confirmed actions
    m_sink_confirmed->handle_actions([this,vg](action::ActionRange range)
    {
        for (uint16_t i=0; i < range.count(); i++)
        {
            auto ar = range.get(i);
            switch (ar.header().meta.type)
            {
                case miro::action::ActionType::Stroke:
                {
                    using Kind = action::StrokeActionRef::Kind;
                    auto a = ar.data<action::StrokeActionRef>();
                    auto uc = get_user_context(a.header().user);
                    if (uc != nullptr) {
                        auto p = a.position();
                        auto pressure = a.pressure();

                        auto idx = a.header().user;
                        p = transform_point(p,uc->stroke_transform(*this));
                        //std::cout << "p " << p.x << ", " << p.y << std::endl;

                        // new stroke building interface
                        if (is_local_user_context(uc)) {
                            if (a.kind() == Kind::Begin) {
                                m_strokes.confirm_stroke(idx);
                            }
                        } else {
                            if (a.kind() == Kind::Update)
                            {
                                m_strokes.add_point(idx,StrokePoint{p,pressure});
                            }
                            else if (a.kind() == Kind::Begin)
                            {
                                m_strokes.begin_stroke(idx,true,StrokeProperties{uc->m_color});
                                m_strokes.add_point(idx,StrokePoint{p,pressure});
                            }
                            else if (a.kind() == Kind::End)
                            {
                                m_strokes.end_stroke(idx);
                            }
                        }

#if 0
                        auto c = uc->m_color;
                        nvgFillColor(vg, nvgRGBA(c.r*255,c.g*255,c.b*255,c.a*255*pressure));
                        nvgBeginPath(vg);
                        //nvgCircle(vg, p.x, p.y, 3);
                        nvgCircle(vg,p.x,p.y,10.0f*pressure);
                        nvgPathWinding(vg, NVG_SOLID);

                        nvgFill(vg);
#endif
                    }
                    break;
                }
                case AT::Viewport:
                {
                    auto a = ar.data<action::ViewportActionRef>();
                    auto uc = get_user_context(a.header().user);
                    if (uc != nullptr) {
                        if(is_local_user_context(uc)) break; // this is already handled when unconfirmed

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
                case AT::Color:
                {
                    auto a = ar.data<action::ColorActionRef>();
                    auto uc = get_user_context(a.header().user);
                    if (uc != nullptr) {
                        if(is_local_user_context(uc)) break; // this is already handled when unconfirmed
                        uc->m_color = a.color();
                    }
                    break;
                }
                default:
                    std::cout << "unhandled action type" << std::endl;
            }
        }
        return true;
    });

    m_render_context.end_frame();

    update_strokes();
}

void Canvas::render(sol::Context &ctx)
{
    auto vg = m_render_context.impl();
    const vec2f fb_dim = (vec2f)m_render_target.dimensions();

    auto uc = get_local_user_context();
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

    // render the overlay RT for temporary / unconfirmed strokes
    // draw a rectangle in canvas space
    nvgResetTransform(vg);
    f = uc->canvas_transform(*this).data();
    nvgTransform(vg, f[0],f[1],f[2],f[3],f[4],f[5]);
    nvgBeginPath(vg);
    nvgRect(vg, 0, 0, 1, 1);

    // fill it with the canvas texture
    nvgResetTransform(vg);
    f = t2.data();
    nvgTransform(vg, f[0],f[1],f[2],f[3],f[4],f[5]);
    nvgFillPaint(vg, m_render_context.nvg_paint(m_render_target_tmp));
    nvgFill(vg);
}

void Canvas::update_strokes()
{
    auto vg = m_render_context.impl();

    // render commited strokes
    m_render_context.begin_frame(m_render_target);
    m_render_context.bind(m_render_target);
    nvgResetTransform(vg);
    nvgReset(vg);

    auto removed = m_strokes.remove_old_strokes();
    for (auto& stroke: removed)
    {
        auto c = stroke.properties().color;
        for (auto& pt : stroke.points())
        {
#if 1
            nvgFillColor(vg, nvgRGBAf(c.r,c.g,c.b,c.a*pt.pressure));
            nvgBeginPath(vg);
            nvgCircle(vg,
                pt.position.x,
                pt.position.y,
                10.0f*pt.pressure);
            nvgPathWinding(vg, NVG_SOLID);
            nvgFill(vg);
#endif
        }

    }
    m_render_context.end_frame();

    // render temporary strokes to overlay render target
    m_render_context.begin_frame(m_render_target_tmp);
    m_render_context.bind(m_render_target_tmp);

    glClearColor(0.0f,0.0f,0.0f,0.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

    for (auto s : m_strokes.strokes())
    {
        auto& stroke = *s;
        auto c = stroke.properties().color;
        for (auto& pt : stroke.points())
        {
#if 1
            nvgFillColor(vg, nvgRGBAf(c.r,c.g,c.b,c.a*pt.pressure));
            nvgBeginPath(vg);
            nvgCircle(vg,
                pt.position.x,
                pt.position.y,
                10.0f*pt.pressure);
            nvgPathWinding(vg, NVG_SOLID);
            nvgFill(vg);
#endif
        }
    }

    m_render_context.end_frame();
}

vec2f Canvas::dimensions() const
{
    return (vec2f)m_render_target.dimensions();
}

UserContext* Canvas::get_user_context(uint16_t idx)
{
    if (idx == m_local_user_idx) return &m_local_user_context;
    if (idx >= m_user_contexts.size()) return nullptr;
    return &m_user_contexts[idx];
}

UserContext *Canvas::get_local_user_context()
{
    return &m_local_user_context;
}

bool Canvas::is_local_user_context(UserContext *uc) const
{
    return uc == &m_local_user_context;
}

void Canvas::handle_user_action(action::UserActionRef &action)
{
    auto idx = action.idx();

    if ((int)action.flags() & (int)action::UserActionRef::Flag::Local) {
        m_local_user_idx = idx;
        m_local_user_context.m_id = idx;
    }
    if (idx >= m_user_contexts.size()) m_user_contexts.resize(idx+1);

    auto uc = get_user_context(idx);

    if (!is_local_user_context(uc)) {
        *uc = UserContext{};
        get_local_user_context()->need_send = true;
    }
    uc->m_alias = std::string(action.alias().ptr(),action.alias().size());
    uc->m_id = idx;

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
