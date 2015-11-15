#include "miro_client.hpp"


#include <iostream>


using namespace networking;

namespace miro
{

Client::Client()
{
    m_client_session.reset(new ClientSession(m_scheduler, m_send_buffer));
    m_send_buffer.set_notify_callback(sol::make_delegate(this,notify_send));
}


Client::~Client()
{
    m_scheduler.stop();
    m_thread.join();
}

bool Client::connect(const char *host, const char *port)
{
    return m_client_session->connect(host,port);
}

void Client::start_thread()
{
    m_thread = std::thread([this](){
        m_scheduler.run();
        std::cout << "Miro::ClientThread finished" << std::endl;
    });
}

IActionSink &Client::send_pipe()
{
    return m_send_buffer;
}

IActionSource &Client::receive_pipe()
{
    return *m_client_session;
}

void Client::notify_send()
{
    m_scheduler.asio().post([this](){
        m_client_session->notify_send_data_available();
    });
}


ClientSession::ClientSession(Scheduler &scheduler, ActionBuffer& send_data)
    : Session(scheduler)
    , m_send_data(send_data)
{
    set_connection_handler(sol::make_delegate(this,connection_handler));
}

void ClientSession::notify_send_data_available()
{
    if (!m_send_active) handle_outgoing();
}

void ClientSession::connection_handler(networking::error_ref e)
{
   if (check_error(e)) {
       handle_incomming();
       handle_outgoing();
   }
}

bool ClientSession::check_error(networking::error_ref e)
{
    if (e) {
        std::cout << "MiroClientSession::error: " << e.message() << std::endl;
        return false;
    }
    return true;
}

void ClientSession::handle_incomming()
{
    connection().receive(&m_receive_header,sizeof(MessageHeader),[this](error_ref e){
        if (check_error(e)) {
            if (m_receive_header.flag == MessageHeader::Flag::action) {
                auto action_count = m_receive_header.len / sizeof(Action);
                // TODO assert that len is a multiple of sizeof(Action)
                receive_actions(action_count);
            }
        }
    });
}

void ClientSession::receive_actions(uint32_t action_count)
{
    m_receive_buffer.resize(action_count);
    connection().receive(m_receive_buffer.data(),action_count*sizeof(Action),[this](error_ref e){
        if (check_error(e)) {
            std::cout << "<received " << m_receive_buffer.size() << " actions>" << std::endl;
            for (auto& a : m_receive_buffer) {
                //IActionSource::send()
                send(a);
            }
            handle_incomming();
        }
    });
}

void ClientSession::handle_outgoing()
{
    uint32_t available = m_send_data.count();
    if (available) {
        m_send_active = true;
        send_action_header();
    } else {
        m_send_active = false;
    }
}

void ClientSession::send_action_header()
{
    m_send_data.get(m_send_data.count(),m_send_buffer);
    m_send_header = MessageHeader();
    m_send_header.flag = MessageHeader::Flag::action;
    m_send_header.len = m_send_buffer.size()*sizeof(Action);
    connection().send(&m_send_header,sizeof(MessageHeader),[this](error_ref e) {
        if (check_error(e)) {
            send_action_data();
        }
    });
}

void ClientSession::send_action_data()
{
    connection().send(m_send_buffer.data(),m_send_buffer.size()*sizeof(Action),[this](error_ref e) {
        if (check_error(e)) {
            std::cout << "<sent " << m_send_buffer.size() << " actions>" << std::endl;
            handle_outgoing();
        }
    });
}

uint32_t ActionBuffer::count()
{
    return m_buffer.size();
}

void ActionBuffer::get(uint32_t count_, std::vector<Action> &output)
{
   std::lock_guard<std::mutex> lock(m_mutex);

   count_ = std::min(count_,count());
   output.clear();
   for (uint32_t i=0; i<count_; i++) {
       output.push_back(m_buffer.front());
       m_buffer.pop_front();
   }
}

void ActionBuffer::set_notify_callback(sol::delegate<void ()> cb)
{
    m_notify_cb = cb;
}

void ActionBuffer::on_receive(Action action)
{
   {
       std::lock_guard<std::mutex> lock(m_mutex);
       m_buffer.push_back(action);
   }
    if (m_notify_cb) m_notify_cb();
}

ActionEchoSession::ActionEchoSession(Connection &&connection)
    : networking::Session(std::move(connection))
{
    set_connection_handler(sol::make_delegate(this,connection_handler));
}

void ActionEchoSession::connection_handler(error_ref e)
{
    if (check_error(e)) {
        receive_action_header();
    }
}

bool ActionEchoSession::check_error(error_ref e)
{
    if (e) {
        std::cout << "ActionEchoSession::error: " << e.message() << std::endl;
        return false;
    }
    return true;
}

void ActionEchoSession::receive_action_header()
{
    connection().receive(&m_receive_header,sizeof(MessageHeader),[this](error_ref e){
        if (check_error(e)) {
            if (m_receive_header.flag == MessageHeader::Flag::action) {
                auto action_count = m_receive_header.len / sizeof(Action);
                // TODO assert that len is a multiple of sizeof(Action)
                receive_actions(action_count);
            }
        }
    });
}

void ActionEchoSession::receive_actions(uint32_t action_count)
{
    m_receive_buffer.resize(action_count);
    connection().receive(m_receive_buffer.data(),action_count*sizeof(Action),[this](error_ref e){
        if (check_error(e)) {
            std::cout << "<received " << m_receive_buffer.size() << " actions>" << std::endl;
            send_action_header();
        }
    });
}

void ActionEchoSession::handle_outgoing()
{

}

void ActionEchoSession::send_action_header()
{
    connection().send(&m_receive_header,sizeof(MessageHeader),[this](error_ref e) {
        if (check_error(e)) {
            send_action_data();
        }
    });
}

void ActionEchoSession::send_action_data()
{
    connection().receive(m_receive_buffer.data(),
                         m_receive_buffer.size()*sizeof(Action),
                         [this](error_ref e){
        if (check_error(e)) {
            std::cout << "<sent " << m_receive_buffer.size() << " actions>" << std::endl;
            receive_action_header();
        }
    });
}

}
