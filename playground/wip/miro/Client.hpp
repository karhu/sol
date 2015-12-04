#pragma once

#include "../common.hpp"

namespace miro {



class Client
{
public:
    Client(const std::string& host) : m_host(host) {}
    ~Client() {}
public:
    int32_t run();
private:
    std::string m_port = "54321";
    std::string m_host = "localhost";
};

}
