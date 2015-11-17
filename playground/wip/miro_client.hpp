#pragma once

//#include <stdint.h>
//#include "vec2.hpp"
//#include <vector>
//#include <deque>
//#include <mutex>

#include "networking/Connection.hpp"
#include "networking/Session.hpp"

#include "Action.hpp"
#include "common.hpp"
#include "delegate.hpp"

#include <mutex>
#include <thread>

namespace miro {

struct MessageHeader {
    enum class Flag : uint8_t {
        ok = 0,
        error = 1,
        action = 2,
    };
    Flag     flag = Flag::ok;
    uint8_t  padding[3];
    uint32_t len = 0;
};

class ConcurrentActionBuffer : public IActionSink {
public:
    uint32_t count();

    void get(uint32_t count, std::vector<Action>& output);
public:
    void set_notify_callback(sol::delegate<void()> cb);

protected:
    virtual void on_receive(Action action) override;
private:
    std::deque<Action> m_buffer;
    std::mutex m_mutex;
    sol::delegate<void()>  m_notify_cb = nullptr;
};

class ClientSession : public networking::Session, public IActionSource {
public:
    ClientSession(networking::Scheduler& scheduler, ConcurrentActionBuffer& send_data);
public:
    void notify_send_data_available();
private:
    void connection_handler(networking::error_ref e);
    bool check_error(networking::error_ref e);
private:
    void handle_incomming();
    void receive_actions(uint32_t action_count);
    void handle_outgoing();
    void send_action_header();
    void send_action_data();
private:
    MessageHeader m_receive_header, m_send_header;
    std::vector<Action> m_receive_buffer;
    std::vector<Action> m_send_buffer;
    ConcurrentActionBuffer& m_send_data;
    bool m_send_active = false;
};

class ActionEchoSession : public networking::Session {
public:
    ActionEchoSession(networking::Connection&& connection);
private:
    void connection_handler(networking::error_ref e);
    bool check_error(networking::error_ref e);
private:
    void receive_action_header();
    void receive_actions(uint32_t action_count);
    void send_action_header();
    void send_action_data();
private:
    MessageHeader m_receive_header, m_send_header;
    std::vector<Action> m_receive_buffer;
    std::vector<Action> m_send_buffer;
};

class Client {
public:
    Client();
    ~Client();
public:
    void start_thread();
    bool connect(const char* host, const char* port);
public:
    IActionSink& send_pipe();
    IActionSource& receive_pipe();
private:
    void notify_send();
private:
    networking::Scheduler m_scheduler;
    ConcurrentActionBuffer m_send_buffer;
    std::unique_ptr<ClientSession> m_client_session;
    std::thread m_thread;
};

}
