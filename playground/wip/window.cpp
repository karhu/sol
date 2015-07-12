#include "window.hpp"
#include "context.hpp"

#include <SDL2/SDL.h>

using namespace arc;

namespace sol {

    WindowHandle WindowSystem::create(uint32_t width, uint32_t height)
    {
        WindowData data;
        data.sdl_window = SDL_CreateWindow(
            "",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            width, height,
            SDL_WINDOW_RESIZABLE);
        uint32_t id = SDL_GetWindowID((SDL_Window*)data.sdl_window);
        m_data.insert(std::make_pair(id,data));
        return WindowHandle(id);
    }

    WindowSystem::WindowSystem(Context &context)
        : m_context(context)
    {}

    Window::~Window() {
        destroy();
    }

    Window::Window(Context &context, uint32_t width, uint32_t height)
        : m_context(context)
    {
        m_sdl_window = SDL_CreateWindow(
            "",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            width, height,
            SDL_WINDOW_RESIZABLE);
    }

    void Window::destroy()
    {
        if (m_sdl_window) {
            m_context.on_destroy_window(*this);
            SDL_DestroyWindow((SDL_Window*)m_sdl_window);
            m_sdl_window = nullptr;
        }
    }

    void Window::ev_close_request() {
        destroy();
    }

    uint32_t Window::get_id() const {
        return SDL_GetWindowID((SDL_Window*)m_sdl_window);
    }
}

