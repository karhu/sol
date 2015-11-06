#pragma once

#include <iostream>
#include <unordered_map>
#include "result.hpp"
#include "vec2.hpp"

namespace sol {

    class Context;

    struct WindowHandle {
    public:
        WindowHandle() : value(0) {}
    private:
        WindowHandle(uint32_t v) : value(v) {}
        uint32_t value;
        friend class WindowSystem;
    };

    class WindowSystem
    {
    public:
        WindowHandle create(uint32_t width, uint32_t height);
        bool valid(WindowHandle h);
        void destroy(WindowHandle h);
    public:
        uint32_t count() const;
        void update();
        void swap(WindowHandle wh);
    public:
        vec2f get_window_dimensions(WindowHandle wh);
        vec2f get_render_target_dimensions(WindowHandle wh);
        WindowHandle get_main_window();
    private:
        struct WindowData
        {
            void* sdl_window = nullptr;
        };
    private:
        WindowSystem(Context& context);
        void ev_close_request(uint32_t window_id);
    private:
        Context& m_context;
        std::unordered_map<uint32_t, WindowData> m_data;
        void* m_gl_context = nullptr;
        uint32_t m_main_window;
    private:
        friend class Context;
        friend class Events;
    };

}

