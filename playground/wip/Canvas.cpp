#include "Canvas.hpp"

#include <iostream>

//#include <GL/gl3w.h>
#include <GL/gl.h>

#include "nanovg.h"
#include "nanovg_gl_utils.h"

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
    static float f = 0.5f;
    static float df = 0.0003f;

    auto& windows = ctx.windows();
    const vec2f win_dim = windows.get_render_target_dimensions(windows.get_main_window());
    const vec2f fb_dim = (vec2f)m_render_target.dimensions();
    auto vg = m_render_context.impl();

    //m_render_context.begin_frame(ctx.windows().get_main_window());
    //m_render_context.bind(sol::RenderTarget::Default);

    m_render_context.begin_frame(m_render_target);
    m_render_context.bind(m_render_target);

    /*
    //clear color and depth buffer
    glClearColor(0.5f,1.0f,0.5f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();//load identity matrix

    //glTranslatef(0.0f,0.0f,-4.0f);//move forward 4 units

    glColor3f(0.0f,0.0f,1.0f); //blue color

    glBegin(GL_LINE_LOOP);//start drawing a line loop
      glVertex3f(-1.0f,0.0f,0.0f);//left of window
      glVertex3f(0.0f,-1.0f,0.0f);//bottom of window
      glVertex3f(1.0f,0.0f,0.0f);//right of window
      glVertex3f(0.0f,1.0f,0.0f);//top of window
    glEnd();//end drawing of line loop

    */
    float target_aspect = fb_dim.x /fb_dim.y;
    float window_aspect = win_dim.x / win_dim.y;

    std::cout << target_aspect << " / " << window_aspect << std::endl;

    vec2f target_dim = {0.8f,0.8f*window_aspect/target_aspect};
    vec2f target_free = vec2f{1,1} - target_dim;
    vec2f target_offset = target_free*0.5f;



    auto count = m_sink_unconfirmed->count();
    for (uint32_t i=0; i<count; i++)
    {
        auto a = m_sink_unconfirmed->pop_front();
        std::cout << a.data.stroke.position.x << ", " << a.data.stroke.position.y << std::endl;

        Transform2f t_fb;
        t_fb.scale = vec2f(1,-1)*(vec2f)fb_dim;
        t_fb.translation = vec2f(0,fb_dim.y);

        auto p = a.data.stroke.position;
        p = p - target_offset;
        p = p / target_dim;

        std::cout << p.x << ", " << p.y << std::endl;

        auto p2 = transform_point(t_fb,p);
        //std::cout << p2.x << ", " << p2.y << std::endl;

        nvgResetTransform(vg);
        nvgReset(vg);
        nvgBeginPath(vg);
        nvgCircle(vg, p2.x, p2.y, 1.5);
        nvgFillColor(vg, nvgRGBA(255,0,0,128));
        nvgFill(vg);

    }

    f += df;
    if (f > 1.01f) {
        f = 1.0f;
        df = -df;
    }
    if (f < -0.01f) {
        f = 0.0f;
        df = -df;
    }



    //glClearColor(f,0.0f,0.0f,1.0f);
    //glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);



    m_render_context.end_frame();



}

void Canvas::render(sol::Context &ctx)
{
    auto& windows = ctx.windows();
    auto win_dim = windows.get_render_target_dimensions(windows.get_main_window());
    vec2f fb_dim = (vec2f)m_render_target.dimensions();
    auto vg = m_render_context.impl();

    m_render_context.begin_frame(ctx.windows().get_main_window());
    m_render_context.bind(sol::RenderTarget::Default);

    static int img = nvgCreateImage(vg, "debug.jpg", 0);
    auto paint = nvgImagePattern(vg, 0,0, 1024,1024,0,img,1.0f);

    vec2f origin = {0.25,0.25};
    vec2f dimension = {0.5,0.5};

    //vec2f target_dim = {800,600};
    //vec2f target_offset = {100,100};

    float target_aspect = fb_dim.x /fb_dim.y;
    float window_aspect = win_dim.x / win_dim.y;

    //std::cout << target_aspect << " / " << window_aspect << std::endl;

    //std::cout << target_aspect << std::endl;

    vec2f target_dim = {0.8f,0.8f*window_aspect/target_aspect};

    //auto tmp = target_dim * win_dim;
    //std::cout << tmp.x / tmp.y << std::endl;

    //std::cout << target_dim.x << " / " << target_dim.y << std::endl;
    vec2f target_free = vec2f{1,1} - target_dim;
    vec2f target_offset = target_free*0.5f;

    //std::cout << target_offset.x << " / " << target_offset.y << std::endl;

    target_dim = target_dim * win_dim;
    target_offset = target_offset * win_dim;



    Transform2f t;
    t.scale = dimension;
    t.translation = origin;
    auto it = inverse(t);

    glClearColor(0.5f,0.5f,0.5f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

    nvgResetTransform(vg);
    nvgBeginPath(vg);
    nvgRect(vg,
            target_offset.x, target_offset.y,
            target_dim.x, target_dim.y);

    nvgTranslate(vg,target_offset.x, target_offset.y);
    nvgScale(vg,
             target_dim.x/m_render_target.dimensions().x,
             target_dim.y/m_render_target.dimensions().y);
    nvgFillPaint(vg, m_render_context.nvg_paint(m_render_target));
    nvgResetTransform(vg);
    nvgFill(vg);
    nvgStrokeColor(vg, nvgRGBA(0,0,0,255));
    nvgStrokeWidth(vg, 2.0f);
    nvgStroke(vg);

    m_render_context.end_frame();


}

}
