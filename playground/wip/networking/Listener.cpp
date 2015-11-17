#include "Listener.hpp"

namespace networking
{
    Listener::Listener(Scheduler &scheduler)
        : m_scheduler(&scheduler)
        , m_acceptor(scheduler.asio())
        , m_next_connection(scheduler)
    {
        set_connection_handler(nullptr);
        set_error_handler(nullptr);
    }

    bool Listener::start(int32_t port)
    {
        asio::error_code ec;

        m_endpoint = asio::ip::tcp::endpoint(asio::ip::tcp::v4(),port);

        m_acceptor.open(m_endpoint.protocol(), ec);
        if (ec) { m_error_handler(ec); return false; }

        m_acceptor.bind(m_endpoint,ec);
        if (ec) { m_error_handler(ec); return false; }

        m_acceptor.listen();

        await_connection();
        return true;
    }

    void Listener::await_connection() {
        m_acceptor.async_accept(
            m_next_connection.m_socket,
            m_next_connection.m_endpoint,
            [this](error_ref e) {
                if (e) {
                    m_error_handler(e);
                } else {
                    m_connection_handler(std::move(m_next_connection));
                    m_next_connection = Connection(*m_scheduler);
                    await_connection();
                }
        });
    }
}
