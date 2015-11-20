#include "Socket.hpp"

namespace networking {

Scheduler &Socket::scheduler()
{
    return *m_scheduler;
}

Socket::Socket(Scheduler &scheduler)
    : m_scheduler(&scheduler)
    , m_socket(scheduler.asio())
{}

Socket::Socket(Socket &&other)
    : m_scheduler(std::move(other.m_scheduler))
    , m_endpoint(std::move(other.m_endpoint))
    , m_socket(std::move(other.m_socket)) 
{

}

Socket& Socket::operator=(Socket&& other)
{
    m_endpoint = std::move(other.m_endpoint);
    m_scheduler = std::move(other.m_scheduler);
    m_socket = std::move(other.m_socket);
    return *this;
}

}
