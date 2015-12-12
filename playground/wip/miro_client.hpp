#pragma once

//#include <stdint.h>
//#include "vec2.hpp"
//#include <vector>
//#include <deque>
//#include <mutex>

#include "networking/Socket.hpp"
#include "networking/Connection.hpp"

#include "Action.hpp"
#include "common.hpp"
#include "delegate.hpp"

#include <mutex>
#include <thread>

namespace miro {

struct MessageHeader {
    enum class Flag : uint8_t {
        undefined = 0,
        ok = 1,
        error = 2,
        action = 3,
    };
    Flag     flag = Flag::ok;
    uint8_t  padding;
    uint16_t len1 = 0;
    uint16_t len2 = 0;
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

class ClientConnection : public networking::Connection, public IActionSource {
public:
    ClientConnection(networking::Scheduler& scheduler, ConcurrentBufferingActionSink& send_data);
public:
    void notify_send_data_available();
private:
    void connection_handler(networking::error_ref e);
    bool check_error(networking::error_ref e);
private:
    void handle_incomming();
    void receive_action_headers();
    void receive_action_data();

    void handle_outgoing();
    void send_action_message();
    void send_action_headers();
    void send_action_data();
private:
    MessageHeader m_receive_header, m_send_header;

    actions::ActionBuffer m_receive_buffer;
    actions::ActionBuffer m_send_buffer;

    ConcurrentBufferingActionSink& m_send_data;
    bool m_send_active = false;
};

class ClientSession {
public:
    ClientSession();
    ~ClientSession();
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
    ConcurrentBufferingActionSink m_send_buffer;
    std::unique_ptr<ClientConnection> m_client_connection;
    std::thread m_thread;
};

}
