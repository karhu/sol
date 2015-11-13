#include "networking.hpp"

#include <iostream>

using namespace asio::ip;

using HandlerFunction = std::function<void(const asio::error_code &ec)>;

namespace networking {

Connection::Connection(Scheduler &scheduler)
    : m_scheduler(&scheduler)
    , m_socket(scheduler.asio())
{}

Connection& Connection::operator=(Connection&& other)
{
    m_endpoint = std::move(other.m_endpoint);
    m_scheduler = std::move(other.m_scheduler);
    m_socket = std::move(other.m_socket);
}

bool Connection::connect(tcp::endpoint endpoint, ConnectCallback cb)
{
    m_endpoint = endpoint;
    m_socket.async_connect(m_endpoint, cb);

    return true;
}

void Connection::send(void* buffer, std::size_t len, SendCallback cb )
{
    asio::async_write(m_socket,asio::buffer(buffer, len), cb);
}

void Connection::receive(void *buffer, std::size_t len, ReceiveCallback cb)
{
    asio::async_read(m_socket,asio::buffer(buffer,len),cb);
}

Scheduler &Connection::scheduler()
{
    return *m_scheduler;
}

Listener::Listener(Scheduler &scheduler, int32_t port)
    : m_scheduler(&scheduler)
    , m_endpoint(tcp::v4(),port)
    , m_acceptor(scheduler.asio())
    , m_next_connection(scheduler)
{}

bool Listener::start()
{
    asio::error_code ec;

    m_acceptor.open(m_endpoint.protocol(), ec);
    if (ec) { std::cout << ec.message() << std::endl; return false; }

    m_acceptor.bind(m_endpoint,ec);
    if (ec) { std::cout << ec.message() << std::endl; return false; }

    m_acceptor.listen();

    HandlerFunction handler = std::bind(
                &Listener::handle_accept, this,
                std::placeholders::_1);

    m_acceptor.async_accept(
                m_next_connection.m_socket,
                m_next_connection.m_endpoint,
                handler);
    return true;
}

void Listener::handle_accept(const asio::error_code &ec)
{
    if (ec) {
        std::cout << ec.message() << std::endl;
    } else {
        std::cout << "new connection" << std::endl;
        //m_active_connections.emplace_back(std::move(m_next_connection));

        m_next_connection.receive(buffer,12,sol::make_delegate(this,&on_receive));

        //m_next_connection = Connection(*m_scheduler);


    }
}

void Listener::on_receive(const asio::error_code &ec, std::size_t count)
{
    if (ec) {
        std::cout << "Listener::on_receive::error " << ec.message() << std::endl;
    } else {
        std::cout << "received: " << buffer << std::endl;
        m_next_connection.send(buffer,12,sol::make_delegate(this,&on_send));
    }
}

void Listener::on_send(const asio::error_code &ec, std::size_t count)
{
    if (ec) {
        std::cout << "Listener::on_send::error " << ec.message() << std::endl;
    } else {
        std::cout << "replied: " << buffer << std::endl;
        m_next_connection.receive(buffer,12,sol::make_delegate(this,&on_receive));
    }
}

void TestProtocol::on_connect(const asio::error_code &ec)
{
    if (ec) {
        std::cout << "TestProtocol::on_connect::error " << ec.message() << std::endl;
    } else {
        std::cout << "TestSession: client connected" << std::endl;

        static char receive_buffer[128];
        static char* message = "hello world";

        m_connection.receive(receive_buffer,12,sol::make_delegate(this,&on_receive));
        m_connection.send(message,12, sol::make_delegate(this,&on_send));
    }
}

void TestProtocol::on_receive(const asio::error_code &ec, std::size_t count)
{
    if (ec) {
        std::cout << "TestProtocol::on_receive::error " << ec.message() << std::endl;
    } else {
        std::cout << "TestSession: received " << count << std::endl;
    }
}

void TestProtocol::on_send(const asio::error_code &ec, std::size_t count)
{
    if (ec) {
        std::cout << "TestProtocol::on_send::error " << ec.message() << std::endl;
    } else {
        std::cout << "TestSession: sent " << count << std::endl;
    }
}

Session2::Session2(Scheduler &scheduler)
    : m_connection(scheduler)
{
    set_error_handler(nullptr);
    set_connection_handler(nullptr);
}

void Session2::connect(const char *host, const char *port)
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
    auto cb = sol::make_delegate(this, &on_connect);
    m_connection.connect(endpoint,cb);
}

void Session2::send(void *data, uint32_t count, Session2::Handler cb)
{
    if (m_handler_send) return; // TODO error
    m_handler_send = cb;
    m_connection.send(data,count,sol::make_delegate(this,on_send));
}

void Session2::receive(void *data, uint32_t count, Session2::Handler cb)
{
    if (m_handler_receive) return; // TODO error
    m_handler_receive = cb;
    m_connection.receive(data,count,sol::make_delegate(this,on_receive));
}

void Session2::set_error_handler(Session2::HandlerError cb)
{
    if (!cb) cb = sol::make_delegate(this,nop_handler_error);
    m_handler_error = cb;
}

void Session2::set_connection_handler(Session2::HandlerConnection cb)
{
    if (!cb) cb = sol::make_delegate(this,nop_handler_connection);
    m_handler_connection = cb;
}

void Session2::on_connect(const asio::error_code &ec)
{
    if (ec) { m_handler_error(ec); return; }
    m_handler_connection();
}

void Session2::on_send(const asio::error_code &ec, std::size_t count)
{
    UNUSED(count);
    auto h = std::move(m_handler_send);
    m_handler_send = Handler(nullptr);
    if (ec) { m_handler_error(ec); return; }
    h();
}

void Session2::on_receive(const asio::error_code &ec, std::size_t count)
{
    UNUSED(count);
    auto h = std::move(m_handler_receive);
    m_handler_receive = Handler(nullptr);
    if (ec) { m_handler_error(ec); return; }
    h();
}

void TestSession::error_handler(const asio::error_code &ec)
{
    std::cout << "TestSession::error_handler: " << ec.message() << std::endl;
}

void TestSession::connection_handler()
{
    std::cout << "TestSession::connected" << std::endl;

    //static char* message = "hello world";
    static char* message = "abrakadabra";
    send(message,12, sol::make_delegate(this,&after_send));
}

void TestSession::after_send()
{
    std::cout << "sent" << std::endl;
    receive(buffer,12, sol::make_delegate(this,&after_receive));
}

void TestSession::after_receive()
{
    std::cout << "received: " << buffer << std::endl;
}

} // networking
