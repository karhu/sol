#pragma once

#include "../common.hpp"
#include "context.hpp"

namespace miro {


struct ClientArgs {
    std::string host = "localhost";
    std::string user_alias = "Joan Miro";
};

class Client
{
public:
    Client(const ClientArgs& args) : m_args(args) {}
    ~Client() {}
public:
    int32_t run();
private:
    void handle_network_interrupt();
private:
    ClientArgs m_args;
    std::string m_port = "54321";
    std::unique_ptr<sol::Context> m_context;
};

}
