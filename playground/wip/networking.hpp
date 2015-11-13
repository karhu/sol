#pragma once

#include "Action.hpp"
#include "common.hpp"
#include "delegate.hpp"

#define ASIO_STANDALONE
#include <asio.hpp>

namespace networking {

class Listener;

/*
class ServerConnection {
public:
    IActionSink& outgoing_actions();
    IActionSource& incomming_actions();
private:
    std::unique_ptr<BufferingActionSink> m_outgoing;
    std::unique_ptr<IActionSource> m_incomming;
};

class Protocol {
    virtual void handle_connect(const asio::error_code& ec, Connection& con ) = 0;
    virtual void handle_read(const asio::error_code& ec, Connection& con ) = 0;
};

*/

class Scheduler {
public:
    inline asio::io_service& asio() { return m_io_service; }
private:
    asio::io_service m_io_service;
};

using ConnectCallback = sol::delegate<void(const asio::error_code& ec)>;
using SendCallback = sol::delegate<void(const asio::error_code& ec, std::size_t transferred)>;
using ReceiveCallback = sol::delegate<void(const asio::error_code& ec, std::size_t transferred)>;

class Connection {
public:
    Connection(Scheduler& scheduler);
public:
    Connection(Connection&& other) = default;
    Connection& operator=(Connection&& other);
public:
    bool connect(asio::ip::tcp::endpoint endpoint, ConnectCallback cb );
    void send(void* buffer, std::size_t len, SendCallback cb );
    void receive(void* buffer, std::size_t len, ReceiveCallback cb);
public:
    Scheduler& scheduler();
private:
    Scheduler* m_scheduler;
    asio::ip::tcp::endpoint m_endpoint;
    asio::ip::tcp::socket m_socket;
private:
    friend class Listener;
};

class Listener {
public:
    Listener(Scheduler& scheduler, int32_t port);
public:
    bool start();
private:
    void handle_accept(const asio::error_code& ec);
private:
    Scheduler* m_scheduler;
    asio::ip::tcp::endpoint m_endpoint;
    asio::ip::tcp::acceptor m_acceptor;
    Connection m_next_connection;
    std::vector<Connection> m_active_connections;
    char buffer[128];
    void on_receive(const asio::error_code& ec, std::size_t count);
    void on_send(const asio::error_code& ec, std::size_t count);
};

template<typename P>
class Session {
public:
    Session(Scheduler& scheduler)
        : m_connection(scheduler)
        , m_protocol(m_connection) {}
    Session(Connection&& connection)
        : m_connection(std::move(connection))
        , m_protocol(m_connection) {}
public:
    void connect(const char* host, const char* port)
    {
        asio::ip::tcp::resolver resolver(m_connection.scheduler().asio());
        asio::ip::tcp::resolver::query query(
            asio::ip::tcp::v4(),
            host,
            port);
        auto iter = resolver.resolve(query);
        asio::ip::tcp::resolver::iterator end;
        if (iter == end) {
            return; // TODO log?
        }
        auto endpoint = *iter;
        auto cb = sol::make_delegate(&m_protocol,&P::on_connect);
        m_connection.connect(endpoint,cb);
    }
public:
    Connection& connection() { return m_connection; }
    P& protocol() { return protocol; }
private:
    Connection m_connection;
    P m_protocol;
};

class Session2 {
public:
    using HandlerError = sol::delegate<void(const asio::error_code& ec)>;
    using HandlerConnection = sol::delegate<void()>;
    using Handler = sol::delegate<void()>;
public:
    Session2(Scheduler& scheduler);
public:
    void connect(const char* host, const char* port);
    void send(void* data, uint32_t count, Handler cb);
    void receive(void* data, uint32_t count, Handler cb);
public:
    void set_error_handler(HandlerError cb);
    void set_connection_handler(HandlerConnection ch);
private:
    void on_connect(const asio::error_code& ec);
    void on_send(const asio::error_code& ec, std::size_t count);
    void on_receive(const asio::error_code& ec, std::size_t count);
private:
    void nop_handler_error(const asio::error_code& ec) {}
    void nop_handler_connection() {}
private:
    Connection m_connection;
    HandlerConnection m_handler_connection = nullptr;
    HandlerError m_handler_error = nullptr;
    Handler m_handler_send = nullptr;
    Handler m_handler_receive = nullptr;
};

class TestSession : public Session2 {
public:
    TestSession(Scheduler& scheduler)
        : Session2(scheduler) {
        set_error_handler(sol::make_delegate(this,error_handler));
        set_connection_handler(sol::make_delegate(this,connection_handler));
    }
protected:
    void error_handler(const asio::error_code& ec);
    void connection_handler();
protected:
    void after_send();
    void after_receive();
    char buffer[128];
};

class TestProtocol {
public:
    TestProtocol(Connection& connection) : m_connection(connection) {}
protected:
    void on_connect(const asio::error_code& ec);
    void on_receive(const asio::error_code& ec, std::size_t count);
    void on_send(const asio::error_code& ec, std::size_t count);
private:
    Connection& m_connection;
private:
    friend class Session<TestProtocol>;
};


}
