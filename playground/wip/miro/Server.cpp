#include "Server.hpp"

#include <iostream>

#include "miro/action/connect.hpp"

using namespace networking;

namespace miro {

Server::Server(uint16_t port)
    : m_port(port)
    , m_listener(m_scheduler)
{

}

void Server::run()
{
    m_listener.set_error_handler([](error_ref e){
       std::cout << "Listener::Error: " << e.message() << std::endl;
    });
    m_listener.set_connection_handler([this](networking::Socket&& socket){
        std::cout << "<new connection> " << std::endl;
        m_only_session.add_connection(std::move(socket));
    });
    m_listener.start(m_port);

    m_scheduler.run();
}

void ServerSession::add_connection(Socket &&socket)
{
    m_connections.emplace_back(new ServerConnection(*this, std::move(socket)));
    auto& con = m_connections.back();
    connect(con->receive_pipe(),m_action_pipe);
    connect(m_action_pipe,con->send_pipe());
}

uint32_t ServerSession::get_next_user_id()
{
    return m_next_user_id++;
}

// ServerConnection /////////////////////////////////

ServerConnection::ServerConnection(ServerSession& session, Socket &&socket)
    : networking::Connection(std::move(socket))
    , m_session(session)
{
    set_connection_handler(sol::make_delegate(this,connection_handler));
    m_send_pipe.set_notify_callback(sol::make_delegate(this,notify_send));

    miro::action::connect(receive_pipe(),send_pipe()); //TODO remove
}

void ServerConnection::connection_handler(error_ref e)
{
    if (check_error(e)) {
        m_user_id = m_session.get_next_user_id();
        handle_incomming();
    }
}

bool ServerConnection::check_error(error_ref e)
{
    if (e) {
        std::cout << "ServerConnection::error: " << e.message() << std::endl;
        return false;
    }
    return true;
}

void ServerConnection::handle_incomming()
{
    //std::cout << "<S><waiting to receive header>"<< std::endl;
    socket().receive(buffer(m_receive_header),[this](error_ref e){
        if (check_error(e)) {
            if (m_receive_header.flag == MessageHeader::Flag::action) {
                auto headers_size = m_receive_header.len1;
                auto data_size = m_receive_header.len2;
                receive_action_headers(headers_size, data_size);
            } else {
                std::cout << "warning: unsupported MessageHeader" << std::endl;
            }
        }
    });
}

void ServerConnection::receive_action_headers(uint16_t len_headers, uint16_t len_data)
{
    auto header_count = len_headers / sizeof(action::ActionHeader);
    len_headers = header_count * sizeof(action::ActionHeader);
    // TODO make sure no rounding happens

    m_buffer_receive.m_headers.resize(header_count);
    //std::cout << "<C><waiting to receive actions>"<< std::endl;
    socket().receive(m_buffer_receive.m_headers.data(),len_headers,[=](error_ref e){
        if (check_error(e)) {
            // std::cout << "<C>< received " << len_headers / sizeof(actions::ActionHeader) << " action headers>" << std::endl;
            receive_action_data(len_headers,len_data);
        }
    });
}

void ServerConnection::receive_action_data(uint16_t len_headers, uint16_t len_data)
{
    // std::cout << "<C><waiting to receive actions>"<< std::endl;
    m_buffer_receive.m_front = len_data;
    socket().receive(m_buffer_receive.m_data.data(),len_data,[=](error_ref e){
        if (check_error(e)) {
            // std::cout << "<C>< received " << len_data << " bytes of action data>" << std::endl;
            m_receive_pipe.send(m_buffer_receive.all());
            m_buffer_receive.reset();
            handle_incomming();
        }
    });
}

void ServerConnection::send_action_message()
{
    m_send_header = MessageHeader();
    m_send_header.flag = miro::MessageHeader::Flag::action;
    m_send_header.len1 = m_buffer_send.size_headers();
    m_send_header.len2 = m_buffer_send.size_data();

    socket().send(buffer(m_send_header),[this](error_ref e) {
        if (check_error(e)) {
            send_action_headers();
        }
    });
}

void ServerConnection::send_action_headers()
{
    socket().send((void*)m_buffer_send.ptr_headers(),m_buffer_send.size_headers(),[this](error_ref e) {
        if (check_error(e)) {
            std::cout << "<S><sent " << m_buffer_send.size_headers() / sizeof(action::ActionHeader) << " action headers>" << std::endl;
            send_action_data();
        }
    });
}

void ServerConnection::send_action_data()
{
    socket().send((void*)m_buffer_send.ptr_data(),m_buffer_send.size_data(),[this](error_ref e) {
        if (check_error(e)) {
            // std::cout << "<S><sent " << m_buffer_send.size_data() << " bytes of action data>" << std::endl;
            handle_outgoing();
        }
    });
}

void ServerConnection::handle_outgoing()
{
    m_buffer_send.reset();
    m_send_pipe.handle_actions([this](action::ActionRange range) {
       m_buffer_send.copy_action(range);
       return false;
    });
    if (m_buffer_send.count() == 0) {
        m_send_active = false;
        return;
    }
    m_send_active = true;
    send_action_message();
}

void ServerConnection::notify_send()
{
    if (!m_send_active) handle_outgoing();
}



}