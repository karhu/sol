#include "miro_client.hpp"


#include <iostream>


using namespace networking;

namespace miro
{

ClientSession::ClientSession()
{
    m_client_connection.reset(new ClientConnection(m_scheduler, m_send_buffer));
    m_send_buffer.set_notify_callback(sol::make_delegate(this,notify_send));
}


ClientSession::~ClientSession()
{
    m_scheduler.stop();
    m_thread.join();
}



bool ClientSession::connect(const char *host, const char *port)
{
    return m_client_connection->connect(host,port);
}

void ClientSession::start_thread()
{
    m_thread = std::thread([this](){
        m_scheduler.run();
        std::cout << "Miro::ClientThread finished" << std::endl;
    });
}

action::IActionSink &ClientSession::send_pipe()
{
    return m_send_buffer;
}

action::IActionSource &ClientSession::receive_pipe()
{
    return *m_client_connection;
}

void ClientSession::notify_send()
{
    m_scheduler.asio().post([this](){
        m_client_connection->notify_send_data_available();
    });
}

// -- ClientSession ------------------ //

ClientConnection::ClientConnection(Scheduler &scheduler, action::ConcurrentBufferingActionSink &send_data)
    : Connection(scheduler)
    , m_send_data(send_data)
{
    set_connection_handler(sol::make_delegate(this,connection_handler));
}

void ClientConnection::notify_send_data_available()
{
    if (!m_send_active) handle_outgoing();
}

void ClientConnection::connection_handler(networking::error_ref e)
{
   if (check_error(e)) {
       handle_incomming();
       handle_outgoing();
   }
}

bool ClientConnection::check_error(networking::error_ref e)
{
    if (e) {
        std::cout << "MiroClientSession::error: " << e.message() << std::endl;
        return false;
    }
    return true;
}

void ClientConnection::handle_incomming()
{
    socket().receive(buffer(m_receive_header),[this](error_ref e){
        if (check_error(e)) {
            //std::cout << "<C>< header received " << m_receive_header.len/sizeof(Action) << ", " <<(int) m_receive_header.flag << " >" << std::endl;
            if (m_receive_header.flag == MessageHeader::Flag::action) {
                receive_action_headers();
            } else {
                std::cout << "warning: unsupported MessageHeader" << std::endl;
            }
        }
    });
}

void ClientConnection::receive_action_headers()
{
    uint16_t len_header = m_receive_header.len1;
    auto header_count = len_header / sizeof(action::ActionHeader);
    len_header = header_count * sizeof(action::ActionHeader);
    // TODO make sure no rounding happens

    m_receive_buffer.m_headers.resize(header_count);
    //std::cout << "<C><waiting to receive actions>"<< std::endl;
    socket().receive(m_receive_buffer.m_headers.data(),len_header,[=](error_ref e){
        if (check_error(e)) {
            // std::cout << "<C>< received " << len_header / sizeof(actions::ActionHeader) << " action headers>" << std::endl;
            receive_action_data();
        }
    });
}

void ClientConnection::receive_action_data()
{
    uint16_t len_data = m_receive_header.len2;
    m_receive_buffer.m_front = len_data;
    socket().receive(m_receive_buffer.m_data.data(),len_data,[=](error_ref e){
        if (check_error(e)) {
            // std::cout << "<C>< received " << len_data << "bytes of action data>" << std::endl;
            send(m_receive_buffer.all());
            m_receive_buffer.reset();
            handle_incomming();
        }
    });
}

void ClientConnection::handle_outgoing()
{
    // copy data over
    m_send_buffer.reset();
    m_send_data.handle_actions([this](action::ActionRange range) {
       m_send_buffer.copy_action(range);
       return false;
    });

    // check we have data
    if (m_send_buffer.count() == 0) {
        m_send_active = false;
        return;
    }

    // init sending
    m_send_active = true;
    send_action_message();
}

void ClientConnection::send_action_message()
{
    m_send_header = MessageHeader();
    m_send_header.flag = miro::MessageHeader::Flag::action;
    m_send_header.len1 = m_send_buffer.size_headers();
    m_send_header.len2 = m_send_buffer.size_data();

    socket().send(buffer(m_send_header),[this](error_ref e) {
        if (check_error(e)) {
            send_action_headers();
        }
    });
}

void ClientConnection::send_action_headers()
{
    socket().send((void*)m_send_buffer.ptr_headers(),m_send_buffer.size_headers(),[this](error_ref e) {
        if (check_error(e)) {
            std::cout << "<C><sent " << m_send_buffer.size_headers() / sizeof(action::ActionHeader) << " action headers>" << std::endl;
            send_action_data();
        }
    });
}

void ClientConnection::send_action_data()
{
    socket().send((void*)m_send_buffer.ptr_data(),m_send_buffer.size_data(),[this](error_ref e) {
        if (check_error(e)) {
            // std::cout << "<C><sent " << m_send_buffer.size_data() << " bytes of action data>" << std::endl;
            handle_outgoing();
        }
    });
}

}
