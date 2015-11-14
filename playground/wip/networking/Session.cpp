#include "Session.hpp"

namespace networking {

Session::Session(Scheduler &scheduler)
    : m_connection(scheduler)
{
    set_connection_handler(nullptr);
}

Session::Session(Connection &&connection)
    : m_connection(std::move(connection))
{
    set_connection_handler(nullptr);
    m_connection.scheduler().asio().post([this](){
        error e;
        m_handler_connection(e);
    });
}

bool Session::connect(const char *host, const char *port)
{
    asio::ip::tcp::resolver resolver(m_connection.scheduler().asio());
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
    m_connection.connect(endpoint,[this](error_ref e){
        m_handler_connection(e);
    });
    return true;
}

void Session::set_connection_handler(Session::ConnectionHandler cb)
{
    if (!cb) cb = sol::make_delegate(this,nop_handler_connection);
    m_handler_connection = cb;
}

}
