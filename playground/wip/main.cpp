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

#include "miro/Server.hpp"

class MainWindow : public sol::EventHandler {
public:
    virtual void handle_cursor_event(const sol::CursorEvent &event) {
        m_canvas_view.handle_cursor_event(event);
    }

public:
    CanvasView m_canvas_view;
    Transform2f m_canvas_transform;
};

struct ClientArgs {
    std::string port = "54321";
    std::string host = "localhost";
};

int client_main(const ClientArgs& args);

int main(int argc, char* argv[])
{
    std::cout << "<start>" << std::endl;

    // read command line arguments ///////
    bool arg_server = false;
    ClientArgs client_args;
    try {
        TCLAP::CmdLine cmd("Command description message", ' ', "0.9");
        TCLAP::SwitchArg server_switch("s","server","run server", cmd, false);
        TCLAP::ValueArg<std::string> host_arg("a","address","host address", false,"localhost","string");
        cmd.add(host_arg);

        // Parse the argv array.
        cmd.parse( argc, argv );

        arg_server = server_switch.getValue();
        client_args.host = host_arg.getValue();

    } catch (TCLAP::ArgException &e) {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    }
    int return_code = 0;

    // run server /////////////////////////
    if (arg_server) {
        const uint16_t port = 54321;
        miro::Server server(port);
        server.run();
    }
    // run client /////////////////////////
    else {
        return_code = client_main(client_args);
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
