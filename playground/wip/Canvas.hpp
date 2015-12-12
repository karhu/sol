#pragma once

#include <stdint.h>
#include <vector>
#include <memory>

#include "Action.hpp"
#include "render_context.hpp"

#include "Transform2.hpp"

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
public:
    void _handle_unconfirmed(actions::ActionRange range);
private:
    struct UserContext {
        Transform2f m_view;
        std::string m_name;
    };
private:
    UserContext* get_user_context(uint16_t id);
    void init_user_context(uint16_t id, const UserContext& context);

private:
    std::unique_ptr<BufferingActionSink> m_sink_unconfirmed;
    std::unique_ptr<BufferingActionSink> m_sink_confirmed;

    std::vector<UserContext> m_user_contexts;
    uint16_t local_user_id = 0;

    sol::RenderContext& m_render_context;
    sol::RenderTarget m_render_target;
    vec2f m_position = {0.5,0.5};
    float m_rotation = 30.0f;
    float m_scale = 1.0f;

    Transform2 m_transform_winr_canvasa; // transform from relative window coordinates to absolute canvas ones
private:
    friend class SinkUnconfirmed;
};

}
