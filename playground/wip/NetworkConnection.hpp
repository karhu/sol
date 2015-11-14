#pragma once

#include "common.hpp"
#include "delegate.hpp"
#include "networking/Session.hpp"


class EchoSession : public networking::Session {
public:
    EchoSession(networking::Scheduler& scheduler)
        : Session(scheduler) {
        set_connection_handler(sol::make_delegate(this,connection_handler));
    }
    EchoSession(networking::Connection&& con)
        : Session(std::move(con)) {
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

class TestSession : public networking::Session {
public:
    TestSession(networking::Scheduler& scheduler)
        : Session(scheduler) {
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

