#pragma once

#include "Action.hpp"
#include "common.hpp"

#define ASIO_STANDALONE
#include <asio.hpp>

namespace networking {

/*
class ServerConnection {
public:
    IActionSink& outgoing_actions();
    IActionSource& incomming_actions();
private:
    std::unique_ptr<BufferingActionSink> m_outgoing;
    std::unique_ptr<IActionSource> m_incomming;
};
*/

class Scheduler {
public:
    inline asio::io_service& asio() { return m_io_service; }
private:
    asio::io_service m_io_service;
};

namespace server {

    class Listener;
    class Session;

    class Session {
    public:
        Session(Scheduler& scheduler);
        Session(Session&& other) = default;
        Session& operator=(Session&& other);
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
        Session m_next_session;
        std::vector<Session> m_active_sessions;
    };
}

namespace client {

    class Session {
    public:
        Session(Scheduler& scheduler, asio::ip::tcp::endpoint endpoint);
    public:
        bool connect();
    private:
        void handle_connect(const asio::error_code& ec);
    private:
        Scheduler* m_scheduler;
        asio::ip::tcp::endpoint m_endpoint;
        asio::ip::tcp::socket m_socket;
    };
}

}
