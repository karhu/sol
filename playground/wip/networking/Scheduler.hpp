#pragma once

#define ASIO_HAS_THREADS
#define ASIO_HAS_STD_MUTEX_AND_CONDVAR
#define ASIO_STANDALONE
#include <asio.hpp>


namespace networking {

class Scheduler {
public:
    ~Scheduler();
public:
    inline asio::io_service& asio() { return m_io_service; }

    void run(); // blocking call, runs until stop() is called
    void stop();
private:

    asio::io_service m_io_service;
    std::unique_ptr<asio::io_service::work> m_work;
};



}
