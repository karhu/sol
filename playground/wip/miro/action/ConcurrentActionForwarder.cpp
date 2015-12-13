#include "ConcurrentActionForwarder.hpp"

namespace miro { namespace action {

    void ConcurrentActionForwarder::poll()
    {
        m_current_buffer->count();
        handle_actions([this](ActionRange range){
           if (range.count() > 0)
               send(range);
           return true;
        });
    }



}}
