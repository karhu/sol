#pragma once

#include "common.hpp"
#include "vec2.hpp"
#include "context.hpp"
#include "window.hpp"

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

    void* _impl = nullptr;
    uint32_t _width = 0;
    uint32_t _height = 0;
};

RenderContext create_render_context();

}
