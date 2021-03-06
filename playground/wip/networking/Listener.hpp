#pragma once

#define ASIO_HAS_THREADS
#define ASIO_HAS_STD_MUTEX_AND_CONDVAR
#define ASIO_STANDALONE
#include <asio.hpp>

#include <functional>

#include "Socket.hpp"

namespace networking {

using error = asio::error_code;
using error_ref = const error&;

class Listener {
public:
    using ErrorHandler = std::function<void(error_ref e)>;
    using ConnectionHandler = std::function<void(Socket&& c)>;
public:
    Listener(Scheduler& scheduler);
public:
    bool start(int32_t port);
public:
    template<typename CB>
    void set_error_handler(CB cb)
    {
        auto fn = ErrorHandler(cb);
        if (!fn) fn = std::bind(&Listener::error_handler_nop,this,std::placeholders::_1);
        m_error_handler = fn;
    }

    template<typename CB>
    void set_connection_handler(CB cb)
    {
        auto fn = ConnectionHandler(cb);
        if (!fn) fn = std::bind(&Listener::connection_handler_nop,this,std::placeholders::_1);
        m_connection_handler = fn;
    }

private:
    void error_handler_nop(error_ref e) { UNUSED(e); }
    void connection_handler_nop(Socket&& c) { UNUSED(c); }
    void await_connection();
private:
    Scheduler* m_scheduler;
    asio::ip::tcp::endpoint m_endpoint;
    asio::ip::tcp::acceptor m_acceptor;
    Socket m_next_connection;
private:
    ErrorHandler m_error_handler = nullptr;
    ConnectionHandler m_connection_handler = nullptr;
};

}
