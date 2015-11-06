#include "window.hpp"
#include "context.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

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
            SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
        uint32_t id = SDL_GetWindowID((SDL_Window*)data.sdl_window);
        m_data.insert(std::make_pair(id,data));

        if (m_gl_context == nullptr) {
            m_gl_context = SDL_GL_CreateContext(  (SDL_Window*)data.sdl_window );
            if( m_gl_context == nullptr )
            {
                printf( "OpenGL context could not be created! SDL Error: %s\n", SDL_GetError() );
                return WindowHandle();
            }
        }

       if (m_data.size() == 1) m_main_window = id;

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

    void WindowSystem::swap(WindowHandle wh)
    {
        auto& data = m_data[wh.value];
        SDL_GL_SwapWindow((SDL_Window*)data.sdl_window);
    }

    vec2f WindowSystem::get_window_dimensions(WindowHandle wh)
    {
        int w,h;
        auto& data = m_data[wh.value];
        SDL_GetWindowSize((SDL_Window*)data.sdl_window, &w, &h);
        return vec2f{w,h};
    }

    vec2f WindowSystem::get_render_target_dimensions(WindowHandle wh)
    {

        int w,h;
        auto& data = m_data[wh.value];
        SDL_GL_GetDrawableSize((SDL_Window*)data.sdl_window, &w, &h);
        return vec2f{w,h};
    }

    WindowHandle WindowSystem::get_main_window()
    {
        return m_main_window;
    }

    WindowSystem::WindowSystem(Context &context)
        : m_context(context)
    {}

    void WindowSystem::ev_close_request(uint32_t window_id)
    {
        auto& data = m_data[window_id];
        SDL_DestroyWindow((SDL_Window*)data.sdl_window);
        m_data.erase(window_id);
        if (window_id == m_main_window) m_main_window = 0;
    }


}

