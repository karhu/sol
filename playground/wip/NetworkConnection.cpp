#include "NetworkConnection.hpp"

#include <iostream>

using namespace networking;

void TestSession::connection_handler(error_ref e)
{
    if (check_error(e)) {
        std::cout << "TestSession::connected" << std::endl;
        send_message();
    }
}

bool TestSession::check_error(error_ref e)
{
    if (e) {
        std::cout << "error: " << e.message() << std::endl;
        return false;
    }
    return true;
}

void TestSession::send_message()
{
    static char* message = "abrakadabra";
    connection().send(message,12, [this] (error_ref e){
        if (check_error(e)) {
            std::cout << "sent" << std::endl;
            receive_message();
        }
    });
}

void TestSession::receive_message()
{
    connection().receive(buffer,12, [this](error_ref e){
       if (check_error(e)) {
            std::cout << "received: " << buffer << std::endl;
       }
    });
}

void EchoSession::connection_handler(error_ref e)
{
    std::cout << "EchoSession::connected" << std::endl;
    if (check_error(e)) receive_message();
}

bool EchoSession::check_error(error_ref e)
{
    if (e) {
        std::cout << "EchoSession::error: " << e.message() << std::endl;
        return false;
    }
    return true;
}

void EchoSession::send_message()
{
    connection().send(buffer,12,[this](error_ref e){
       if (check_error(e)) {
           std::cout << "sent: " << buffer << std::endl;
           receive_message();
       }
    });
}

void EchoSession::receive_message()
{
    connection().receive(buffer,12, [this](error_ref e){
       if (check_error(e)) {
            std::cout << "received: " << buffer << std::endl;
            send_message();
       }
    });
}
