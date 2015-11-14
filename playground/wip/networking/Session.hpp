#pragma once

#define ASIO_STANDALONE
#include <asio.hpp>

#include "Connection.hpp"
#include "../delegate.hpp"

namespace networking {

class Session {
public:
    using ConnectionHandler = sol::delegate<void(const asio::error_code& ec)>;
public:
    Session(Scheduler& scheduler);
    Session(Connection&& connection);
public:
    bool connect(const char* host, const char* port);
    Connection& connection() { return m_connection; }
protected:
    void set_connection_handler(ConnectionHandler ch);
private:
    void nop_handler_connection(error_ref e) { UNUSED(e); }
private:
    Connection m_connection;
    ConnectionHandler m_handler_connection = nullptr;
};

}
