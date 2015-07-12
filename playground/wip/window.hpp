#pragma once

#include <iostream>
#include <unordered_map>
#include "result.hpp"

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
    private:
        struct WindowData
        {
            void* sdl_window = nullptr;
        };
    private:
        WindowSystem(Context& context);
    private:
        Context& m_context;
        std::unordered_map<uint32_t, WindowData> m_data;
    private:
        friend class Context;
    };

    class Window
    {
    public:
        ~Window();
    public:
        bool valid() { return m_sdl_window != nullptr; }
        uint32_t get_id() const;
    protected:
        Window(Context& context, uint32_t width, uint32_t height);
    private:
        Window(const Window& other) = delete;
        Window& operator= (const Window& other) = delete;
    private:
        void destroy();
        void ev_close_request();
    private:
        int m_int;
        Context& m_context;
        void* m_sdl_window = nullptr;
    private:
        friend class Events;
    };

}

