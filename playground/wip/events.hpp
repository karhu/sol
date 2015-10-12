#pragma once

#include <stdint.h>
#include "vec2.hpp"

namespace sol {

    class Context;

    struct CursorEvent {
        enum class Type : uint8_t {
            Mouse,
            Touch,
            Stylus,
        };

        enum class Action : uint8_t {
            Up,
            Down,
            Move,
        };

        Type  type;
        float pressure;
        vec2f position;
    };

    class Events {
    public:
        void update();
        /* blocking version of update() */
        void wait();
    public:
        bool should_quit() const { return m_should_quit; }

    public:
        void emit_should_quit();
    private:
        Events(Context& context);
        ~Events();
    private:
        bool m_should_quit = false;
        Context& m_context;
    private:
        friend class Context;
    };

}
