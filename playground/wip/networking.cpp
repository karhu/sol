#include "networking.hpp"

#include <iostream>

using namespace asio::ip;

using HandlerFunction = std::function<void(const asio::error_code &ec)>;

namespace networking { namespace server {

Listener::Listener(Scheduler &scheduler, int32_t port)
    : m_scheduler(&scheduler)
    , m_endpoint(tcp::v4(),port)
    , m_acceptor(scheduler.asio())
    , m_next_session(scheduler)
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
                m_next_session.m_socket,
                m_next_session.m_endpoint,
                handler);
    return true;
}

void Listener::handle_accept(const asio::error_code &ec)
{
    if (ec) {
        std::cout << ec.message() << std::endl;
    } else {
        std::cout << "new connection" << std::endl;
        m_active_sessions.emplace_back(std::move(m_next_session));
        m_next_session = Session(*m_scheduler);
    }
}

Session::Session(Scheduler &scheduler)
    : m_scheduler(&scheduler)
    , m_socket(scheduler.asio())
{}

Session& Session::operator=(Session&& other)
{
    m_endpoint = std::move(other.m_endpoint);
    m_scheduler = std::move(other.m_scheduler);
    m_socket = std::move(other.m_socket);
}

} // server

namespace client {

Session::Session(Scheduler &scheduler, tcp::endpoint endpoint)
    : m_scheduler(&scheduler)
    , m_endpoint(endpoint)
    , m_socket(scheduler.asio())
{}

bool Session::connect()
{
    auto handler = std::bind(
                &Session::handle_connect,this,
                std::placeholders::_1);

    m_socket.async_connect(m_endpoint, handler);

    return true;
}

void Session::handle_connect(const asio::error_code &ec)
{
    if (ec) {
        std::cout << ec.message() << std::endl;
    } else {
        std::cout << "client connected" << std::endl;
    }
}

} // client

} // networking
