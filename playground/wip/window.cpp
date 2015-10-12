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

    uint32_t WindowSystem::count() const
    {
        return m_data.size();
    }

    void WindowSystem::update()
    {
        if (m_data.size() == 0) {
            m_context.events().emit_should_quit();
        }
    }

    WindowSystem::WindowSystem(Context &context)
        : m_context(context)
    {}

    void WindowSystem::ev_close_request(uint32_t window_id)
    {
        auto& data = m_data[window_id];
        SDL_DestroyWindow((SDL_Window*)data.sdl_window);
        m_data.erase(window_id);
    }


}

