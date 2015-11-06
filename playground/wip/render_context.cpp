#include "render_context.hpp"

#include "common.hpp"

#include <memory>

#include <GL/gl3w.h>
#include <GL/gl.h>

#include "nanovg.h"
#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg_gl.h"
#include "nanovg_gl_utils.h"

namespace sol {

RenderContext create_render_context(sol::Context& ctx)
{
    RenderContext context;
    context.init(ctx);
    return std::move(context);
}

RenderContext::~RenderContext()
{
    reset();
}

void RenderContext::init(sol::Context& context)
{
    m_context = &context;

    static bool gl_init = false;

    if (!gl_init) {
        if (gl3wInit()) {
            fprintf(stderr, "failed to initialize OpenGL\n");
            return;
        }
        if (!gl3wIsSupported(3, 2)) {
            fprintf(stderr, "OpenGL 3.2 not supported\n");
            return;
        }
        gl_init = true;
    }

    if (!_impl) _impl = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
}

bool RenderContext::valid() const
{
    return _impl != nullptr;
}

void RenderContext::reset()
{
    if (_impl) {
        nvgDeleteGL3((NVGcontext*)_impl);
        _impl = nullptr;
    }
}

void RenderContext::bind(RenderTarget &target)
{
    nvgluBindFramebuffer((NVGLUframebuffer*)target._impl);
}

RenderTarget RenderContext::create_render_target(uint32_t w, uint32_t h, RTFlags flags)
{
    RenderTarget t;
    auto vg = (NVGcontext*)_impl;
    t._impl = nvgluCreateFramebuffer(vg, w, h, (int)flags);
    t._width = w;
    t._height = h;
    return std::move(t);
}

bool RenderContext::begin_frame(RenderTarget& target, float pixelDensity)
{
    auto vg = (NVGcontext*)_impl;
    vec2f dim;

    if (&target == &RenderTarget::Default) {
        return false;
    } else {
        dim = vec2f(target._width,target._height);
    }

    nvgBeginFrame(vg, dim.x, dim.y, pixelDensity);
    glViewport(0, 0, dim.x, dim.y);
    return true;
}

bool RenderContext::begin_frame(sol::WindowHandle wh)
{
    auto vg = (NVGcontext*)_impl;
    auto wd = m_context->windows().get_window_dimensions(wh);
    auto wrtd = m_context->windows().get_render_target_dimensions(wh);
    //std::cout << wd.x << " / " << wd.y << " / " << wrtd.x << " / " << wrtd.y << std::endl;
    nvgBeginFrame(vg, wd.x, wd.y, wrtd.x/wd.x);
    glViewport(0, 0, wrtd.x, wrtd.y);
    return true;
}

void RenderContext::end_frame()
{
    auto vg = (NVGcontext*)_impl;
    nvgEndFrame(vg);
}

RenderContext::RenderContext(RenderContext && other)
{
    _impl = other._impl;
    other._impl = nullptr;
}

RenderContext &RenderContext::operator=(RenderContext && other)
{
    reset();
    _impl = other._impl;
    other._impl = nullptr;
    return *this;
}

RenderTarget RenderTarget::Default = RenderTarget();

RenderTarget::~RenderTarget()
{
    reset();
}

RenderTarget::RenderTarget(RenderTarget && other)
{
    _impl = other._impl;
    other._impl = nullptr;

    _width = other._width;
    _height = other._height;
}

RenderTarget &RenderTarget::operator=(RenderTarget && other)
{
    reset();
    _impl = other._impl;
    other._impl = nullptr;

    _width = other._width;
    _height = other._height;

    return *this;
}

bool RenderTarget::valid() const
{
    return _impl != nullptr;
}

void RenderTarget::reset()
{
    if (_impl) {
        nvgluDeleteFramebuffer((NVGLUframebuffer*) _impl);
        _impl = nullptr;

        _width = 0;
        _height = 0;
    }
}

}
