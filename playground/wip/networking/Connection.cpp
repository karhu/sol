#include "Connection.hpp"

namespace networking {

Scheduler &Connection::scheduler()
{
    return *m_scheduler;
}

Connection::Connection(Scheduler &scheduler)
    : m_scheduler(&scheduler)
    , m_socket(scheduler.asio())
{}

Connection::Connection(Connection &&other)
    : m_scheduler(std::move(other.m_scheduler))
    , m_socket(std::move(other.m_socket))
    , m_endpoint(std::move(other.m_endpoint))
{

}

Connection& Connection::operator=(Connection&& other)
{
    m_endpoint = std::move(other.m_endpoint);
    m_scheduler = std::move(other.m_scheduler);
    m_socket = std::move(other.m_socket);
    return *this;
}

}
