#include <iostream>
#include <SDL2/SDL.h>

#include "context.hpp"
#include "window.hpp"

#include "VideoPlaybackTest.hpp"

#include "Canvas.hpp"
#include "CanvasView.hpp"
#include "render_context.hpp"

#include "miro_client.hpp"
#include "networking/Listener.hpp"

#include "memory.hpp"
#include "memory/Mallocator.hpp"

#include <tclap/CmdLine.h>

class MainWindow : public sol::EventHandler {
public:
    virtual void handle_cursor_event(const sol::CursorEvent &event) {
        m_canvas_view.handle_cursor_event(event);
    }

public:
    CanvasView m_canvas_view;
    Transform2f m_canvas_transform;
};

struct ServerArgs {
    int32_t port = 54321;
};

struct ClientArgs {
    std::string port = "54321";
    std::string host = "localhost";
};

int client_main(const ClientArgs& args);
int server_main(const ServerArgs& args);

int main(int argc, char* argv[])
{
    std::cout << "<start>" << std::endl;

    // read command line arguments
    bool arg_server = false;
    try {
        TCLAP::CmdLine cmd("Command description message", ' ', "0.9");
        TCLAP::SwitchArg server_switch("s","server","run server", cmd, false);

        // Parse the argv array.
        cmd.parse( argc, argv );

        arg_server = server_switch.getValue();

    } catch (TCLAP::ArgException &e) {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    }
    int return_code = 0;
    if (arg_server) {
        ServerArgs args;
        return_code = server_main(args);
    } else {
        ClientArgs args;
        return_code = client_main(args);
    }

    std::cout << "<end>" << std::endl;

    return return_code;
}

using namespace networking;

int client_main(const ClientArgs& args)
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

    std::cout << std::endl;

    miro::Client client;
    client.connect(args.host.c_str(),args.port.c_str());
    client.start_thread();

    connect(main_window.m_canvas_view,client.send_pipe());

    miro::ConcurrentActionForwarder incomming_buffer;
    connect(client.receive_pipe(),incomming_buffer);
    connect(incomming_buffer,canvas.sink_unconfirmed());

    while (!ctx.events().should_quit()) {
        ctx.events().update();
        incomming_buffer.poll();
        canvas.update(ctx);
        canvas.render(ctx);
        ctx.windows().swap(wh);
    }

    return 0;
}

int server_main(const ServerArgs& args)
{

    networking::Scheduler scheduler;
    networking::Listener listener(scheduler);
    std::vector<miro::ActionEchoSession> echo_sessions;
    listener.set_error_handler([](error_ref e){
       std::cout << "Listener::Error: " << e.message() << std::endl;
    });
    listener.set_connection_handler([&](networking::Connection&& con){
        std::cout << "<new connection> " << std::endl;
        echo_sessions.emplace_back(std::move(con));
    });
    listener.start(args.port);

    while(true) {
        auto count = scheduler.asio().poll();
        if (count >0) std::cout << count << std::endl;
    }


    return 0;
}
