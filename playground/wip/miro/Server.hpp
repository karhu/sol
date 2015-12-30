#pragma once

#include "common.hpp"
#include "networking/Listener.hpp"
#include "networking/Scheduler.hpp"
#include "miro_client.hpp"

#include "miro/action/NotifyingActionBuffer.hpp"
#include "miro/action/ActionSender.hpp"
#include "miro/action/ActionForwarder.hpp"

namespace miro {

class ServerSession;

class ServerConnection : public networking::Connection {
public:
    ServerConnection(ServerSession& session, networking::Socket&& socket);
public:
    action::IActionSink& send_pipe() { return m_send_pipe; }
    action::IActionSource& receive_pipe() { return m_receive_pipe; }
private:
    void connection_handler(networking::error_ref e);
    bool check_error(networking::error_ref e);
private:
    void handle_handshake();

    void handle_incomming();
    void receive_action_headers(uint16_t len_header, uint16_t len_data);
    void receive_action_data(uint16_t len_header, uint16_t len_data);

    void send_action_message();
    void send_action_headers();
    void send_action_data();
    void handle_outgoing(bool reset = true);

    void notify_send();
private:
    ServerSession& m_session;

    MessageHeader m_receive_header, m_send_header;
    action::ActionBuffer m_buffer_receive;
    action::ActionBuffer m_buffer_send;

    action::ActionSender m_receive_pipe;
    action::NotifyingActionBuffer m_send_pipe;

    std::vector<char> m_tmp_buffer;

private:
    bool m_send_active = false;
    uint32_t m_user_id = -1;
};

class ServerSession {
public:
    void add_connection(networking::Socket&& socket);
    uint32_t get_next_user_id();
    action::ActionForwarder& action_multiplexer();
private:
    std::string m_name;
    std::vector<std::unique_ptr<ServerConnection>> m_connections;
    action::ActionForwarder m_action_pipe;
    uint32_t m_next_user_id = 1;
};

class Server {
    NO_COPY(Server)
public:
    Server(uint16_t port);
    void run();
private:
    uint16_t m_port;
    networking::Scheduler m_scheduler;
    networking::Listener  m_listener;

    ServerSession m_only_session;
};

}
