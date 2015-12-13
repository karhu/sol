#pragma once

namespace miro { namespace action {

    enum class ActionType : uint8_t{
        Unknown = 0,
        Stroke,
        Viewport,
        User,
    };

}}
