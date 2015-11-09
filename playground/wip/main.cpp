#include <iostream>
#include <SDL2/SDL.h>

#include "context.hpp"
#include "window.hpp"

#include "VideoPlaybackTest.hpp"

#include "Canvas.hpp"
#include "CanvasView.hpp"
#include "render_context.hpp"

class MainWindow : public sol::EventHandler {
public:
    virtual void handle_cursor_event(const sol::CursorEvent &event) {
        m_canvas_view.handle_cursor_event(event);
    }

public:
    CanvasView m_canvas_view;
    Transform2f m_canvas_transform;
};

int main(int argc, char* argv[])
{
    UNUSED(argc);
    UNUSED(argv);

    std::cout << "<start>" << std::endl;

    auto context = sol::get_context();
    auto& ctx = *context;

    auto wh = ctx.windows().create(900,600);

    sol::RenderContext render_context;
    render_context.init(ctx);

    /*
    vec2f origin = {0.5,1.0};
    vec2f dimension = {0.5,-0.5};

    Transform2f t;
    t.scale = dimension;
    t.translation = origin;
    auto it = inverse(t);
    */

    MainWindow main_window;
    miro::Canvas canvas(render_context,1200,900);
    main_window.m_canvas_view.set_canvas(canvas);
    ctx.events().register_handler(main_window);

    while (!ctx.events().should_quit()) {
        ctx.events().update();
        canvas.update(ctx);
        canvas.render(ctx);
        ctx.windows().swap(wh);
    }

    std::cout << "<end>" << std::endl;

    return 0;
}
