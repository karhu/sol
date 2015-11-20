#include "Server.hpp"

#include <iostream>

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
}

void ServerConnection::connection_handler(error_ref e)
{
    if (check_error(e)) {
        m_user_id = m_session.get_next_user_id();
        receive_action_header();
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

void ServerConnection::receive_action_header()
{
    //std::cout << "<S><waiting to receive header>"<< std::endl;
    socket().receive(buffer(m_receive_header),[this](error_ref e){
        if (check_error(e)) {
            //std::cout << "<C>< header received " << m_receive_header.len/sizeof(Action) << ", " <<(int) m_receive_header.flag << " >" << std::endl;
            if (m_receive_header.flag == MessageHeader::Flag::action) {
                auto action_count = m_receive_header.len / sizeof(Action);
                m_receive_buffer.resize(action_count);
                // TODO assert that len is a multiple of sizeof(Action)
                receive_actions();
            } else {
                std::cout << "warning: unsupported MessageHeader" << std::endl;
            }
        }
    });
}

void ServerConnection::receive_actions()
{
    //std::cout << "<S><waiting to receive actions>"<< std::endl;
    socket().receive(buffer(m_receive_buffer),[this](error_ref e){
        if (check_error(e)) {
            std::cout << "<S><received " << m_receive_buffer.size() << " actions>" << std::endl;

            for (auto& a : m_receive_buffer) {
                //a.data.stroke.position = a.data.stroke.position + vec2f{0.1f,0.1f};
                m_receive_pipe.send(a);
            }
            receive_action_header();
        }
    });
}

void ServerConnection::send_action_header()
{
    m_send_header.flag = miro::MessageHeader::Flag::action;
    m_send_header.len = buffer(m_send_buffer).size();
    socket().send(buffer(m_send_header),[this](error_ref e) {
        if (check_error(e)) {
            send_action_data();
        }
    });
}

void ServerConnection::send_action_data()
{
    socket().send(buffer(m_send_buffer),[this](error_ref e) {
        if (check_error(e)) {
            if (m_send_pipe.count() > 0) {
                send_actions();
            } else {
                m_send_active = false;
            }
        }
    });
}

void ServerConnection::send_actions()
{
    m_send_active = true;
    m_send_pipe.get(m_send_buffer);
    send_action_header();
}

void ServerConnection::notify_send()
{
    if (!m_send_active) send_actions();
}

// NotifyingActionBuffer /////////////////////////////////

uint32_t NotifyingActionBuffer::count()
{
    return m_buffer.size();
}

void NotifyingActionBuffer::get(std::vector<Action> &output, uint32_t count)
{
    count = std::min(count,this->count());
    output.resize(count);
    for (auto& a: output) {
        a = m_buffer.front();
        m_buffer.pop_front();
    }
}

void NotifyingActionBuffer::set_notify_callback(sol::delegate<void ()> cb)
{
    m_notify_cb = cb;
}

void NotifyingActionBuffer::on_receive(Action action)
{
    m_buffer.push_back(action);
    if (m_notify_cb) m_notify_cb();
}



}
