#pragma once

#include "../common.hpp"
#include "../networking/Listener.hpp"
#include "../networking/Scheduler.hpp"
#include "../miro_client.hpp"

namespace miro {

class ServerSession;

class NotifyingActionBuffer : public IActionSink {
public:
    uint32_t count();
    void get(std::vector<Action>& output, uint32_t count = -1);
public:
    void set_notify_callback(sol::delegate<void()> cb);

protected:
    virtual void on_receive(Action action) override;
private:
    std::deque<Action> m_buffer;
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
    void receive_action_header();
    void receive_actions();
    void send_action_header();
    void send_action_data();
    void send_actions();
    void notify_send();
private:
    ServerSession& m_session;
    MessageHeader m_receive_header, m_send_header;
    std::vector<Action> m_receive_buffer;
    std::vector<Action> m_send_buffer;
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
