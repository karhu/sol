#pragma once

#define ASIO_STANDALONE
#include <asio.hpp>

namespace networking {

class Scheduler {
public:
    inline asio::io_service& asio() { return m_io_service; }
private:
    asio::io_service m_io_service;
};

}
