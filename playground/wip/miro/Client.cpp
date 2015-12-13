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
    virtual void handle_cursor_event(const sol::CursorEvent &event) {
        m_canvas_view.handle_cursor_event(event);
    }

public:
    CanvasView m_canvas_view;
    Transform2f m_canvas_transform;
};

int32_t miro::Client::run()
{
    auto context = sol::get_context();
    auto& ctx = *context;

    auto wh = ctx.windows().create(900,600);

    sol::RenderContext render_context;
    render_context.init(ctx);

    MainWindow main_window;
    miro::Canvas canvas(render_context,1200,900);
    main_window.m_canvas_view.set_canvas(canvas);
    ctx.events().register_handler(main_window);

    miro::ClientSession session;
    session.connect(m_host.c_str(),m_port.c_str());
    session.start_thread();

    connect(main_window.m_canvas_view.get_action_source(),session.send_pipe());

    action::ConcurrentActionForwarder incomming_buffer;
    connect(session.receive_pipe(),incomming_buffer);
    connect(incomming_buffer,canvas.sink_confirmed());

    while (!ctx.events().should_quit()) {
        ctx.events().update();
        incomming_buffer.poll();
        canvas.update(ctx);
        canvas.render(ctx);
        ctx.windows().swap(wh);
    }
    return 0;
}

}
