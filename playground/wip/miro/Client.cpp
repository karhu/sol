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
    miro::Canvas canvas(render_context,1200,900);
    main_window.m_canvas_view.set_canvas(canvas);
    ctx.events().register_handler(main_window);

    miro::ClientSession session(UserInfo{
        m_args.user_alias
    });
    session.connect(m_args.host.c_str(),m_port.c_str());
    session.start_thread();

    connect(main_window.m_canvas_view.get_action_source(),session.send_pipe());

    action::ConcurrentActionForwarder incomming_buffer;
    connect(session.receive_pipe(),incomming_buffer);
    connect(incomming_buffer,canvas.sink_confirmed());

    incomming_buffer.set_notify_callback(sol::make_delegate(this,handle_network_interrupt));

    while (!ctx.events().should_quit()) {
        ctx.events().wait();
        //std::cout << "events " << count << std::endl;
        incomming_buffer.poll();
        canvas.update(ctx);
        canvas.render(ctx);
        ctx.windows().swap(wh);
    }
    return 0;
}

void Client::handle_network_interrupt()
{
    m_context->events().interrupt_wait();
}

}
