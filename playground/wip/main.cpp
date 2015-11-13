#include <iostream>
#include <SDL2/SDL.h>

#include "context.hpp"
#include "window.hpp"

#include "VideoPlaybackTest.hpp"

#include "Canvas.hpp"
#include "CanvasView.hpp"
#include "render_context.hpp"

#include "networking.hpp"
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

class MemoryTest {
public:
    MemoryTest(int i) {
        mi = i;
    }

    ~MemoryTest() {
        std::cout << "~MemoryTest(" << mi << ")" << std::endl;
    }

    void inc() { mi++; }

    int mi;
    int mi2;
};

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

    sol::memory::Mallocator mallocator;

    sol::memory::rich_ptr<MemoryTest> obj = mallocator.create<MemoryTest>(42);
    obj->inc();

    obj.destroy();

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

    Scheduler scheduler;
    //Session<TestProtocol> session = Session<TestProtocol>(networking::Connection(scheduler));
    //session.connect(args.host.c_str(), args.port.c_str());

    TestSession test_session(scheduler);
    test_session.connect(args.host.c_str(), args.port.c_str());

    while (!ctx.events().should_quit()) {
        ctx.events().update();
        canvas.update(ctx);
        canvas.render(ctx);
        ctx.windows().swap(wh);
        auto count = scheduler.asio().poll();
        if (count >0) std::cout << count << std::endl;
    }

    return 0;
}

int server_main(const ServerArgs& args)
{
    networking::Scheduler scheduler;
    networking::Listener listener(scheduler,args.port);
    listener.start();

    while(true) {
        auto count = scheduler.asio().poll();
        if (count >0) std::cout << count << std::endl;
    }

    return 0;
}
