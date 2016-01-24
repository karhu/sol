#pragma once

#include <stdint.h>
#include <vector>
#include <memory>


#include "render_context.hpp"

#include "Transform2.hpp"
#include "color.hpp"

#include "miro/action/ActionRange.hpp"
#include "miro/action/IActionSink.hpp"

#include "Stroke.hpp"

namespace miro {

namespace action {
    class BufferingActionSink;
    struct UserActionRef;
    struct ViewportActionRef;
}

class Canvas;

struct UserContext
{
    vec2f   m_canvas_position = {.5,.5};
    float   m_canvas_rotation = 0;
    float   m_canvas_scale = 1;
    vec2u16 m_view_dimensions = {100,100};

    sol::color::RGBA m_color = {0,1,1,0.5f};

    std::string m_alias;        // name chosen by the user
    uint16_t    m_id;           // automatically assigned id

public:
    void update_transform(const Canvas &canvas, action::ViewportActionRef& action);
    const Transform2& stroke_transform(const Canvas& canvas);
        // transform from stroke coordinates
        // to canvas coordinates
    const Transform2& canvas_transform(const Canvas& canvas);
private:
    Transform2 m_stroke_transform;
        // transform from stroke coordinates
        // to canvas coordinates
    Transform2 m_canvas_transform;
        // transform from relative canvas coordinates
        // to absolute window coordinates
    bool m_stroke_transform_dirty = true;
    bool m_canvas_transform_dirty = true;

};

class Canvas
{
public:
    Canvas(sol::RenderContext& rctx, uint32_t width, uint32_t height);
    ~Canvas() = default;
public:
    action::IActionSink& sink_unconfirmed();
    action::IActionSink& sink_confirmed();
public:
    void update(sol::Context &ctx);
    void render(sol::Context &ctx);
public:
    vec2f dimensions() const;
public:
    UserContext* get_user_context(uint16_t idx);
    UserContext* get_local_user_context();

    bool is_local_user_context(UserContext* uc) const;
private:
    void handle_user_action(action::UserActionRef& action);
private:
    std::unique_ptr<action::BufferingActionSink> m_sink_unconfirmed;
    std::unique_ptr<action::BufferingActionSink> m_sink_confirmed;

    std::vector<UserContext> m_user_contexts;
    UserContext m_local_user_context;
    uint16_t m_local_user_idx = 0;

    sol::RenderContext& m_render_context;
    sol::RenderTarget m_render_target;

    Transform2 m_transform_winr_canvasa; // transform from relative window coordinates to absolute canvas ones
    miro::StrokeCollection m_strokes;
};

}
