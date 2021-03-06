#pragma once

#include <unordered_map>

#include "core.hpp"
#include "window.hpp"
#include "events.hpp"

namespace sol {

    class Context;

    arc::uptr<Context> get_context();

    class Context {
    public:
        ~Context();
    public:
        Events& events() { return m_events; }
        const Events& events() const { return m_events; }
        WindowSystem& windows() { return m_window_system; }
    protected:
        Context();
        Context(const Context& other) = delete;
        Context& operator= (const Context& other) = delete;
    private:
        bool m_valid = true;
        Events m_events;
        WindowSystem m_window_system;
    private:
        static uint32_t s_counter;
    private:
        friend arc::uptr<Context> get_context();
        friend class Events;
        friend class Window;
    };

}
