#pragma once

#include "../common.hpp"
#include "../networking/Listener.hpp"
#include "../networking/Scheduler.hpp"
#include "../miro_client.hpp"

namespace miro {

class ServerSession;

class NotifyingActionBuffer : public BufferingActionSink {
public:
    void set_notify_callback(sol::delegate<void()> cb);
protected:
    virtual void on_receive(actions::ActionRange range) override;
private:
    sol::delegate<void()>  m_notify_cb = nullptr;
};

class ServerConnection : public networking::Connection {
public:
    ServerConnection(ServerSession& session, networking::Socket&& socket);
public:
    IActionSink& send_pipe() { return m_send_pipe; }
    IActionSource& receive_pipe() { return m_receive_pipe; }
private:
    void connection_handler(networking::error_ref e);
    bool check_error(networking::error_ref e);
private:
    void handle_incomming();
    void receive_action_headers(uint16_t len_header, uint16_t len_data);
    void receive_action_data(uint16_t len_header, uint16_t len_data);

    void send_action_message();
    void send_action_headers();
    void send_action_data();
    void handle_outgoing();

    void notify_send();
private:
    ServerSession& m_session;

    MessageHeader m_receive_header, m_send_header;
    actions::ActionBuffer m_buffer_receive;
    actions::ActionBuffer m_buffer_send;

    ActionSender m_receive_pipe;
    NotifyingActionBuffer m_send_pipe;

private:
    bool m_send_active = false;
    uint32_t m_user_id = -1;
};

class ServerSession {
public:
    void add_connection(networking::Socket&& socket);
    uint32_t get_next_user_id();

private:
    std::string m_name;
    std::vector<std::unique_ptr<ServerConnection>> m_connections;
    ActionForwarder m_action_pipe;
    uint32_t m_next_user_id = 0;
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
