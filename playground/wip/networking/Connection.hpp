#pragma once

#define ASIO_HAS_THREADS
#define ASIO_HAS_STD_MUTEX_AND_CONDVAR
#define ASIO_STANDALONE
#include <asio.hpp>

#include "Socket.hpp"
#include "../delegate.hpp"

namespace networking {

class Connection {
public:
    using ConnectionHandler = sol::delegate<void(const asio::error_code& ec)>;
public:
    Connection(Scheduler& scheduler);
    Connection(Socket&& socket);
public:
    bool connect(const char* host, const char* port);
    Socket& socket() { return m_socket; }
protected:
    void set_connection_handler(ConnectionHandler ch);
private:
    void nop_handler_connection(error_ref e) { UNUSED(e); }
private:
    Socket m_socket;
    ConnectionHandler m_handler_connection = nullptr;
};

}
