#include "NetworkConnection.hpp"

#include <iostream>
#include <array>

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


void MiroServerSession::connection_handler(error_ref e)
{
    if (check_error(e)) {
        init_handshake();
    }
}

bool MiroServerSession::check_error(error_ref e)
{
    if (e) {
        std::cout << "MiroServerSession::error: " << e.message() << std::endl;
        return false;
    }
    return true;
}

void MiroServerSession::init_handshake()
{
    static ServerHandshake handshake;

    connection().send(&handshake,sizeof(ServerHandshake),[this](error_ref e) {
        if (check_error(e)) {
            receive_handshake();
        }
    });
}

void MiroServerSession::receive_handshake()
{
    connection().receive(&m_buffer,sizeof(ClientHandshake),[this](error_ref e){
        if (check_error(e)) {
            auto is = *buffer_get<ClientHandshake>();
            ClientHandshake should;
            if (is.magic == should.magic && is.version == should.version) {
                finish_handshake();
            }else{
                // TODO reply error
            }
        }
    });
}

void MiroServerSession::finish_handshake()
{
    auto data = buffer_emplace<MessageHeader>();
    connection().send(data,sizeof(MessageHeader),[this](error_ref e){
        if (check_error(e)) {
            await_instructions();
        }
    });
}

void MiroServerSession::await_instructions()
{
    // TODO
}

// -- CLIENT --------------------------- //

void MiroClientSession::connection_handler(error_ref e)
{

}

bool MiroClientSession::check_error(error_ref e)
{
    if (e) {
        std::cout << "MiroClientSession::error: " << e.message() << std::endl;
        return false;
    }
    return true;
}

void MiroClientSession::receive_handshake_init()
{
    connection().receive(m_buffer.data(),sizeof(ServerHandshake), [this](error_ref e) {
        if (check_error(e)) {
            auto is = *buffer_get<ServerHandshake>();
            ServerHandshake should;
            if (is.magic == should.magic && is.version == should.version) {
                send_handshake_reply();
            } else {
                // TODO
            }
        }
    });
}

void MiroClientSession::send_handshake_reply()
{
    auto data = buffer_emplace<ClientHandshake>();
    connection().send(data,sizeof(ClientHandshake),[this](error_ref e){
        if (check_error(e)) {
            receive_handshake_finish();
        }
    });
}

void MiroClientSession::receive_handshake_finish()
{
    connection().receive(m_buffer.data(),sizeof(MessageHeader), [this](error_ref e) {
        if (check_error(e)) {
            auto h = *buffer_get<MessageHeader>();
            if (h.flag == MessageHeader::Flag::ok) {
                // TODO
            }
        }
    });
}
