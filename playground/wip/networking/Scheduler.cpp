#include "Scheduler.hpp"

#include <iostream>

namespace networking {

Scheduler::~Scheduler()
{
    stop();
}


void Scheduler::run()
{
    m_work.reset(new asio::io_service::work(m_io_service));
    m_io_service.run();
    std::cout << "Scheduler finished running." << std::endl;
}

void Scheduler::stop()
{
    if (m_work) {
        std::cout << "stop called 1" << std::endl;
        m_io_service.stop();
        m_work.reset(nullptr);
        std::cout << "stop called 2" << std::endl;
    }
}

}
