#pragma once

#include <stdint.h>
#include <vector>
#include <memory>

#include "Action.hpp"
#include "render_context.hpp"

namespace miro {

class Canvas
{
public:
    Canvas(sol::RenderContext& rctx, uint32_t width, uint32_t height);
    ~Canvas() = default;
public:
    IActionSink& sink_unconfirmed();
    IActionSink& sink_confirmed();
public:
    void update(sol::Context &ctx);
    void render(sol::Context &ctx);
//private:
    std::unique_ptr<BufferingActionSink> m_sink_unconfirmed;
    std::unique_ptr<BufferingActionSink> m_sink_confirmed;

    sol::RenderContext& m_render_context;
    sol::RenderTarget m_render_target;
    vec2f m_position = {0.5,0.5};
    float m_rotation = 30.0f;
    float m_scale = 1.0f;
};

}
