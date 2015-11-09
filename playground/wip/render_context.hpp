#pragma once

#include "common.hpp"
#include "vec2.hpp"
#include "context.hpp"
#include "window.hpp"

#include "nanovg.h"
struct NVGLUframebuffer;

namespace sol {

enum class RTFlags: uint32_t {
    None = 0
};

struct RenderTarget;

struct RenderContext
{
    RenderContext() = default;
    ~RenderContext();
    RenderContext(RenderContext const&) = delete;
    RenderContext& operator=(RenderContext const&) = delete;
    RenderContext(RenderContext &&);
    RenderContext& operator=(RenderContext &&);

    void init(sol::Context& context);
    bool valid() const;
    void reset();

    void bind(RenderTarget& target);
    RenderTarget create_render_target(uint32_t w, uint32_t h, RTFlags flags = RTFlags::None);

    bool begin_frame(RenderTarget& target, float pixelDensity = 1.0f);
    bool begin_frame(sol::WindowHandle wh);

    void end_frame();
public:
    inline NVGcontext* impl() { return (NVGcontext*) _impl; }
public:
    NVGpaint nvg_paint(RenderTarget& rt);

public:
    void* _impl = nullptr;
    sol::Context* m_context = nullptr;
};

struct RenderTarget
{
    RenderTarget() = default;
    ~RenderTarget();
    RenderTarget(RenderTarget const&) = delete;
    RenderTarget& operator=(RenderTarget const&) = delete;
    RenderTarget(RenderTarget &&);
    RenderTarget& operator=(RenderTarget &&);

    bool valid() const;
    void reset();

    static RenderTarget Default;
public:
    inline NVGLUframebuffer* impl() { return (NVGLUframebuffer*) m_impl; }
    inline vec2u32 dimensions() { return vec2u32{m_width,m_height}; }
private:
    void* m_impl = nullptr;
    uint32_t m_width = 0;
    uint32_t m_height = 0;
private:
    friend class RenderContext;
};

RenderContext create_render_context();

}
