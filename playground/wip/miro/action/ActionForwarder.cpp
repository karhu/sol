#include "ActionForwarder.hpp"

namespace miro { namespace action {

    void ActionForwarder::on_receive(ActionRange range)
    {
        send(range);
    }

}}



