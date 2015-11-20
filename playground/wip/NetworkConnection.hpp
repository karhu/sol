#pragma once

#include "common.hpp"
#include "delegate.hpp"
#include "networking/Connection.hpp"


class EchoSession : public networking::Connection {
public:
    EchoSession(networking::Scheduler& scheduler)
        : Connection(scheduler) {
        set_connection_handler(sol::make_delegate(this,connection_handler));
    }
    EchoSession(networking::Socket&& sock)
        : Connection(std::move(sock)) {
        set_connection_handler(sol::make_delegate(this,connection_handler));
    }
protected:
    void connection_handler(networking::error_ref e);
private:
    bool check_error(networking::error_ref e);
    void send_message();
    void receive_message();
private:
    char buffer[128];
};

class TestSession : public networking::Connection {
public:
    TestSession(networking::Scheduler& scheduler)
        : Connection(scheduler) {
        set_connection_handler(sol::make_delegate(this,connection_handler));
    }
protected:
    void connection_handler(networking::error_ref e);
private:
    bool check_error(networking::error_ref e);
    void send_message();
    void receive_message();
private:
    char buffer[128];
};

struct ServerHandshake {
    std::array<char,8> magic{{'m','i','r','o','-','s','r','v'}};
    uint32_t version = 1;
    uint32_t zero = 0;
};
struct ClientHandshake {
    std::array<char,8> magic{{'m','i','r','o','-','c','l','i'}};
    uint32_t version = 1;
    uint32_t zero = 0;
};
struct MessageHeader {
    enum class Flag : uint8_t {
        ok = 0,
        error = 1,
    };
    Flag     flag = Flag::ok;
    uint8_t  padding;
    uint8_t  contextual = 0;
    uint32_t len = 0;
};

class MiroServerSession : public networking::Connection {
public:
    MiroServerSession(networking::Socket&& connection)
        : Connection(std::move(connection)) {
        set_connection_handler(sol::make_delegate(this,connection_handler));
    }
private:
    void connection_handler(networking::error_ref e);
private:
    bool check_error(networking::error_ref e);
    void init_handshake();
    void receive_handshake();
    void finish_handshake();
    void await_instructions();
private:
    template<typename T>
    T* buffer_emplace()
    {
        static_assert(sizeof(T) <= BUFFER_SIZE, "insufficient buffer size");
        T* t = (T*)m_buffer.data();
        *t = T();
        return t;
    }
    template<typename T>
    T* buffer_get()
    {
        static_assert(sizeof(T) <= BUFFER_SIZE, "insufficient buffer size");
        T* t = (T*)m_buffer.data();
        return t;
    }

    static constexpr uint32_t BUFFER_SIZE = 128;
    std::array<char,BUFFER_SIZE> m_buffer;
};

class MiroClientSession : public networking::Connection {
public:
    MiroClientSession(networking::Scheduler& scheduler)
        : Connection(scheduler) {
        set_connection_handler(sol::make_delegate(this,connection_handler));
    }
private:
    void connection_handler(networking::error_ref e);
    bool check_error(networking::error_ref e);
private:
    void receive_handshake_init();
    void send_handshake_reply();
    void receive_handshake_finish();
private:
    template<typename T>
    T* buffer_emplace()
    {
        static_assert(sizeof(T) <= BUFFER_SIZE, "insufficient buffer size");
        T* t = (T*)m_buffer.data();
        *t = T();
        return t;
    }
    template<typename T>
    T* buffer_get()
    {
        static_assert(sizeof(T) <= BUFFER_SIZE, "insufficient buffer size");
        T* t = (T*)m_buffer.data();
        return t;
    }

    static constexpr uint32_t BUFFER_SIZE = 128;
    std::array<char,BUFFER_SIZE> m_buffer;
};

