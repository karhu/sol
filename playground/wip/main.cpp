#include <iostream>

#include <tclap/CmdLine.h>

#include "miro/Server.hpp"
#include "miro/Client.hpp"

#include "Action.hpp"

struct ClientArgs {
    std::string port = "54321";
    std::string host = "localhost";
};

void action_test() {
    using namespace miro::v3;

    std::cout << "[ACTION-TEST] start" << std::endl;

    ActionBuffer b;
    bool ok;
    ok = write_stroke_action(b,HeaderData(),vec2f(1,1),1.0f,0,0);
    if (!ok) std::cout << "<<error>> write_stroke_action 1" << std::endl;
    ok = write_message_action(b,HeaderData(),"Hello World!",42);
    if (!ok) std::cout << "<<error>> write_message_action 42" << std::endl;
    ok = write_stroke_action(b,HeaderData(),vec2f(2,2),2.0f,0,0);
    if (!ok) std::cout << "<<error>> write_stroke_action 2" << std::endl;

    StrokeActionRef sa1(b,0);
    if (!sa1.valid()) std::cout << "<<error>> StrokeActionRef 1" << std::endl;
    else
        std::cout << sa1.pressure() << std::endl;
    StrokeActionRef sa2(b,2);
    if (!sa2.valid()) std::cout << "<<error>> StrokeActionRef 2" << std::endl;
    else
        std::cout << sa2.pressure() << std::endl;
    MessageActionRef ma(b,1);
    if (!ma.valid()) std::cout << "<<error>> StrokeActionRef 2" << std::endl;
    else {
        std::cout << std::string(ma.message().m_front,ma.message().m_end) << std::endl;
    }

    std::cout << "[ACTION-TEST] end" << std::endl;
}

int main(int argc, char* argv[])
{
    std::cout << "<start>" << std::endl;

    // read command line arguments ///////
    bool arg_server = false;
    ClientArgs client_args;
    try {
        TCLAP::CmdLine cmd("Command description message", ' ', "0.9");
        TCLAP::SwitchArg server_switch("s","server","run server", cmd, false);
        TCLAP::ValueArg<std::string> host_arg("a","host","host address", false,"localhost","string");
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
        miro::Client client(client_args.host);
        return_code = client.run();
    }

    action_test();

    std::cout << "<end>" << std::endl;
    return return_code;
}
