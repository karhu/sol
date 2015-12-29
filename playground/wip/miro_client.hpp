#pragma once

//#include <stdint.h>
//#include "vec2.hpp"
//#include <vector>
//#include <deque>
//#include <mutex>

#include "networking/Socket.hpp"
#include "networking/Connection.hpp"

#include "common.hpp"
#include "delegate.hpp"

#include <mutex>
#include <thread>

#include "miro/action/IActionSource.hpp"
#include "miro/action/ConcurrentBufferingActionSink.hpp"

namespace miro {

struct MessageHeader {
    enum class Flag : uint8_t {
        undefined = 0,
        ok = 1,
        error = 2,
        action = 3,
        handshake = 4,
    };
    Flag     flag = Flag::ok;
    uint8_t  padding;
    uint16_t len1 = 0;
    uint16_t len2 = 0;
};

struct UserInfo {
    std::string user_alias;
};

class ClientSession;

class ClientConnection : public networking::Connection, public action::IActionSource {
public:
    ClientConnection(networking::Scheduler& scheduler, ClientSession& session, action::ConcurrentBufferingActionSink& send_data);
public:
    void notify_send_data_available();
private:
    void connection_handler(networking::error_ref e);
    bool check_error(networking::error_ref e);
private:
    void handle_handshake();

    void handle_incomming();
    void receive_action_headers();
    void receive_action_data();

    void handle_outgoing();
    void send_action_message();
    void send_action_headers();
    void send_action_data();
private:
    MessageHeader m_receive_header, m_send_header;

    action::ActionBuffer m_receive_buffer;
    action::ActionBuffer m_send_buffer;

    action::ConcurrentBufferingActionSink& m_send_data;
    bool m_send_active = false;

    ClientSession& m_session;
};

class ClientSession {
public:
    ClientSession(const UserInfo& user_info);
    ~ClientSession();
public:
    void start_thread();
    bool connect(const char* host, const char* port);
public:
    action::IActionSink& send_pipe();
    action::IActionSource& receive_pipe();
private:
    void notify_send();
private:
    UserInfo m_user_info;
    networking::Scheduler m_scheduler;
    action::ConcurrentBufferingActionSink m_send_buffer;
    std::unique_ptr<ClientConnection> m_client_connection;
    std::thread m_thread;
private:
    friend class ClientConnection;
};

}
