#include "Canvas.hpp"

#include <iostream>

#include "nanovg.h"

#include <GL/gl3w.h>
#include <GL/gl.h>

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

Canvas::Canvas()
{
    m_sink_confirmed.reset(new SinkConfirmed(*this));
    m_sink_unconfirmed.reset(new SinkUnconfirmed(*this));
}

IActionSink &Canvas::sink_unconfirmed()
{
    return *m_sink_unconfirmed;
}

IActionSink &Canvas::sink_confirmed()
{
    return *m_sink_confirmed;
}

bool Canvas::init(sol::Context& context, uint32_t width, uint32_t height)
{
    m_render_context.init(context);
    m_context = &context;
    //m_render_target = m_render_context.create_render_target(width,height);
    //m_render_context.bind(m_render_target);

    return true;
}

void Canvas::update()
{
    static float f = 0.5f;
    static float df = 0.0003f;

    m_render_context.begin_frame(m_context->windows().get_main_window());
    auto vg = (NVGcontext*)m_render_context._impl;

    auto& windows = m_context->windows();
    auto fb_dim = windows.get_render_target_dimensions(windows.get_main_window());

    auto count = m_sink_unconfirmed->count();
    for (uint32_t i=0; i<count; i++)
    {
        auto a = m_sink_unconfirmed->pop_front();
        std::cout << df << " " << a.data.stroke.position.x << ", " << a.data.stroke.position.y << std::endl;

        auto p = a.data.stroke.position * fb_dim;

        nvgBeginPath(vg);
        nvgCircle(vg, p.x, p.y, 3);
        nvgFillColor(vg, nvgRGBA(255,0,0,128));
        nvgFill(vg);
    }

    m_render_context.bind(sol::RenderTarget::Default);

    f += df;
    if (f > 1.01f) {
        f = 1.0f;
        df = -df;
    }
    if (f < -0.01f) {
        f = 0.0f;
        df = -df;
    }



    //glClearColor(0.5f,0.5f,0.5f,1.0f);
    //glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);


    nvgResetTransform(vg);
    nvgBeginPath(vg);
    nvgRect(vg, 100,100, 120,30);
    //nvgCircle(vg, 120,20, 5);
    //nvgPathWinding(vg, NVG_HOLE);   // Mark circle as a hole.
    nvgFillColor(vg, nvgRGBA(0,192,255,255));
    nvgFill(vg);

    m_render_context.end_frame();
}

}
