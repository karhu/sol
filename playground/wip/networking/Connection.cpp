#include "Connection.hpp"

namespace networking {

Connection::Connection(Scheduler &scheduler)
    : m_socket(scheduler)
{
    set_connection_handler(nullptr);
}

Connection::Connection(Socket &&connection)
    : m_socket(std::move(connection))
{
    set_connection_handler(nullptr);
    m_socket.scheduler().asio().post([this](){
        error e;
        m_handler_connection(e);
    });
}

bool Connection::connect(const char *host, const char *port)
{
    asio::ip::tcp::resolver resolver(m_socket.scheduler().asio());
    asio::ip::tcp::resolver::query query(
        asio::ip::tcp::v4(),
        host,
        port);
    auto iter = resolver.resolve(query);
    asio::ip::tcp::resolver::iterator end;
    if (iter == end) {
        return false; // TODO log?
    }
    auto endpoint = *iter;
    m_socket.connect(endpoint,[this](error_ref e){
        m_handler_connection(e);
    });
    return true;
}

void Connection::set_connection_handler(Connection::ConnectionHandler cb)
{
    if (!cb) cb = sol::make_delegate(this,nop_handler_connection);
    m_handler_connection = cb;
}

}
