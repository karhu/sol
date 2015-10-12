#pragma once

#include <stdint.h>
#include <vector>

class Canvas
{
public:
    Canvas();
    ~Canvas();
private:
    struct Action {
        enum class Type : uint16_t {
            Unknown = 0,
            DrawBegin,
            DrawMove,
            DrawEnd,
            ChangeView,
        };

        Type type;

    };
};
