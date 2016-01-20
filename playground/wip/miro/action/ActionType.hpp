#pragma once

namespace miro { namespace action {

    enum class ActionType : uint8_t{
        Unknown = 0,
        Stroke,         // draw a stroke
        Viewport,       // change viewport transform
        User,           // user update
        Color,
        Message,
    };

}}
