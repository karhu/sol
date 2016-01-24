#include "Client.hpp"

#include <iostream>
#include <SDL2/SDL.h>

#include "../context.hpp"
#include "../window.hpp"

#include "../Canvas.hpp"
#include "../CanvasView.hpp"
#include "../render_context.hpp"

#include "../miro_client.hpp"
#include "../networking/Listener.hpp"

#include "../memory.hpp"
#include "../memory/Mallocator.hpp"

#include "miro/action/ConcurrentActionForwarder.hpp"

#include <GL/gl.h>
#include "nanovg.h"
#include "nanovg_gl_utils.h"


namespace miro {

class MainWindow : public sol::EventHandler {
public:
    MainWindow(sol::Context& ctx)
        : m_canvas_view(ctx)
    {}

    virtual void handle_cursor_event(const sol::CursorEvent &event) override {
        m_canvas_view.handle_cursor_event(event);
    }

    virtual void handle_window_event(const sol::WindowEvent& event) override {
        m_canvas_view.handle_window_event(event);
    }

    virtual void handle_keyboard_event(const sol::KeyboardEvent &event) override {
        m_canvas_view.handle_keyboard_event(event);
    }

    void render(sol::Context& ctx, sol::RenderContext& rctx)
    {
        rctx.begin_frame(ctx.windows().get_main_window());
        rctx.bind(sol::RenderTarget::Default);

        // clear the view
        glClearColor(0.75f,0.75f,0.75f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

        m_canvas_view.render(ctx, rctx);

        rctx.end_frame();
    }

public:
    CanvasView m_canvas_view;
};

int32_t miro::Client::run()
{
    m_context = sol::get_context();
    auto& ctx = *m_context;

    auto wh = ctx.windows().create(900,600);

    sol::RenderContext render_context;
    render_context.init(ctx);

    MainWindow main_window(ctx);
    ctx.events().register_handler(main_window);

    miro::Canvas canvas(render_context,1200,900);

    miro::ClientSession session(UserInfo{
        m_args.user_alias
    });
    session.connect(m_args.host.c_str(),m_port.c_str());

    action::ConcurrentActionForwarder incomming_buffer;
    incomming_buffer.set_notify_callback(sol::make_delegate(this,handle_network_interrupt));

    connect(main_window.m_canvas_view.get_action_source(),session.send_pipe());
    connect(session.receive_pipe(),incomming_buffer);
    connect(incomming_buffer,canvas.sink_confirmed());

    main_window.m_canvas_view.set_canvas(canvas);
    session.start_thread();

    while (!ctx.events().should_quit()) {
        ctx.events().wait();
        incomming_buffer.poll();
        canvas.update(ctx);
        main_window.render(ctx,render_context);
        ctx.windows().swap(wh);
    }
    return 0;
}

void Client::handle_network_interrupt()
{
    m_context->events().interrupt_wait();
}

}
