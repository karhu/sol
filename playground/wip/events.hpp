#pragma once

#include <stdint.h>
#include <vector>
#include "vec2.hpp"
#include "common.hpp"

namespace sol {

    class Context;
    class Events;

    struct CursorEvent
    {
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
        Action action;
        uint8_t button;
        float pressure;
        uint32_t timestamp;
        vec2f position;
        vec2f velocity;
    };

    class EventHandler
    {
    public:
        virtual ~EventHandler();
        virtual void handle_cursor_event(const CursorEvent& event) { UNUSED(event); }
    private:
        Events* m_source = nullptr;
    private:
        friend class Events;
    };

    class Events
    {
    public:
        void update();
        /* blocking version of update() */
        void wait();
    public:
        bool register_handler(EventHandler& handler);
        bool unregister_handler(EventHandler& handler);
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
        std::vector<EventHandler*> m_handlers;
    private:
        friend class Context;
    };

}
