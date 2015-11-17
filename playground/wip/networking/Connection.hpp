#pragma once

#include "../common.hpp"
#include "Scheduler.hpp"

#include <iostream>

namespace networking {

using error = asio::error_code;
using error_ref = const error&;

class Connection {
public:
    Connection(Scheduler& scheduler);
public:
    Connection(Connection&& other);
    Connection& operator=(Connection&& other);
public:
    template<typename CB>
    bool connect(asio::ip::tcp::endpoint endpoint, CB cb ) {
        m_endpoint = endpoint;
        m_socket.async_connect(m_endpoint, cb);
        return true;
    }

    template<typename CB>
    bool send(void* buffer, std::size_t len, CB cb ) {
        if (m_sending) return false;
        //std::cout << "[send " << len << "]" << std::endl;
        m_sending = true;
        asio::async_write(
            m_socket,
            asio::buffer(buffer, len),
            [this, cb](const asio::error_code& ec, std::size_t transferred){
                UNUSED(transferred);
                m_sending = false;
                cb(ec);
        });
        return true;
    }

    template<typename CB>
    bool receive(void* buffer, std::size_t len, CB cb) {
        if (m_receiving) return false;
        //std::cout << "[recv " << len << "]" << std::endl;
        m_receiving = true;
        asio::async_read(
            m_socket,
            asio::buffer(buffer,len),
            [this, cb](const asio::error_code& ec, std::size_t transferred) {
                UNUSED(transferred);
                m_receiving = false;
                cb(ec);
        });
        return true;
    }

public:
    Scheduler& scheduler();
private:
    Scheduler* m_scheduler;
    asio::ip::tcp::endpoint m_endpoint;
    asio::ip::tcp::socket m_socket;
private:
    bool m_sending = false;
    bool m_receiving = false;
private:
    friend class Listener;
};

}
