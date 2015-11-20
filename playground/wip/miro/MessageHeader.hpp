#pragma once

#include "../common.hpp"

namespace miro
{
    struct MessageHeader {
        enum class Flag : uint8_t {
            ok = 0,
            error = 1,
            action = 2,
            handshake = 109,
        };
        Flag     flag = Flag::ok;
        char     padding[3] = {'i','r','o'};
        uint32_t len = 0;
    };

}

