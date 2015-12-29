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

    struct WindowEvent
    {
        enum class Type : uint8_t {
            Resize,
            Close
        };

        Type type;
        vec2f size;
    };

    class EventHandler
    {
    public:
        virtual ~EventHandler();
        virtual void handle_cursor_event(const CursorEvent& event) { UNUSED(event); }
        virtual void handle_window_event(const WindowEvent& event) { UNUSED(event); }
    private:
        Events* m_source = nullptr;
    private:
        friend class Events;
    };

    class Events
    {
    public:
        void update();
        /* blocking version of update(), handles at least some events before returning */
        uint32_t wait();
        void interrupt_wait();
    public:
        bool register_handler(EventHandler& handler);
        bool unregister_handler(EventHandler& handler);
    public:
        bool should_quit() const { return m_should_quit; }
    public:
        void emit_should_quit();
    private:
        void handle_sdl_event(void* sdl_event);
    private:
        Events(Context& context);
        ~Events();
    private:
        bool m_should_quit = false;
        Context& m_context;
        std::vector<EventHandler*> m_handlers;

        uint32_t m_custom_event;
    private:
        friend class Context;
    };

}
