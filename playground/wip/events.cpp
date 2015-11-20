#include "events.hpp"

#include <iostream>
#include <SDL2/SDL.h>
#include "context.hpp"

namespace sol {
    Events::Events(Context& context)
        : m_context(context)
    {}

    Events::~Events()
    {

    }

    void Events::update()
    {
        uint32_t counter = 0;
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            counter += 1;
            //std::cout << "e: " << e.type << std::endl;
            switch (e.type) {
                case SDL_QUIT: {
                    m_should_quit = true;
                    break;
                }
                case SDL_WINDOWEVENT: {
                    auto ew = e.window;
                    //std::cout << "ew: " << ew.type << std::endl;
                    switch (ew.event) {
                        case SDL_WINDOWEVENT_CLOSE:
                            m_context.m_window_system.ev_close_request(ew.windowID);
                            break;
                    }
                    break;
                }
                case SDL_FINGERMOTION :
                case SDL_FINGERUP:
                case SDL_FINGERDOWN: {
                    auto ew = e.tfinger;

                    CursorEvent ce;
                    ce.position = vec2f{ew.x,1.0f-ew.y};
                    ce.velocity = vec2f{ew.dx,-ew.dy};
                    ce.timestamp = ew.timestamp;
                    ce.pressure= ew.pressure;
                    ce.type = CursorEvent::Type::Stylus;
                    ce.action = e.type == SDL_FINGERMOTION ? CursorEvent::Action::Move :
                                e.type == SDL_FINGERDOWN   ? CursorEvent::Action::Down :
                                CursorEvent::Action::Up;
                    ce.button = 1;
                    for (auto eh : m_handlers) { eh->handle_cursor_event(ce); }
                    break;
                }
                case SDL_MOUSEMOTION: {
                    auto ew = e.motion;
                    if (ew.which == SDL_TOUCH_MOUSEID) break;

                    auto window = SDL_GetWindowFromID(ew.windowID);
                    int ww, wh;
                    SDL_GetWindowSize(window, &ww, &wh);

                    CursorEvent ce;
                    ce.position = vec2f{(float)ew.x/ww,1.0f - (float)ew.y/wh};
                    ce.velocity = vec2f{(float)ew.xrel/ww,-(float)ew.yrel/wh};
                    ce.timestamp = ew.timestamp;
                    ce.type = CursorEvent::Type::Mouse;
                    ce.action = CursorEvent::Action::Move;
                    ce.button = 0;
                    for (auto eh : m_handlers) { eh->handle_cursor_event(ce); }
                    break;
                }
                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEBUTTONUP: {
                    auto ew = e.button;
                    if (ew.which == SDL_TOUCH_MOUSEID) break;

                    auto window = SDL_GetWindowFromID(ew.windowID);
                    int ww, wh;
                    SDL_GetWindowSize(window, &ww, &wh);

                    CursorEvent ce;
                    ce.position = vec2f{(float)ew.x/ww,1.0f-(float)ew.y/wh};
                    ce.velocity = vec2f{0,0};
                    ce.timestamp = ew.timestamp;
                    ce.type = CursorEvent::Type::Mouse;
                    ce.action = ew.type == SDL_MOUSEBUTTONDOWN ?
                                CursorEvent::Action::Down :
                                CursorEvent::Action::Up;
                    ce.button = ew.button; // 1,2,3
                    // ew.clicks
                    for (auto eh : m_handlers) { eh->handle_cursor_event(ce); }
                    break;
                }
            }
        }
        //std::cout << "event_counter: " << counter << std::endl;
    }

    bool Events::register_handler(EventHandler &handler)
    {
        if (handler.m_source != nullptr) return false;
        handler.m_source = this;
        m_handlers.push_back(&handler);
        return true;
    }

    bool Events::unregister_handler(EventHandler& handler)
    {
        if (handler.m_source != this) return false;

        for (uint32_t i=0; i<m_handlers.size(); i++) {
            if (m_handlers[i] == &handler) {
                m_handlers[i] = m_handlers.back();
                m_handlers.pop_back();
            }
            handler.m_source = nullptr;
            return true;
        }

        return false;
    }

    void Events::emit_should_quit()
    {
        m_should_quit = true;
    }

    void wait()
    {
        // TODO
    }

    EventHandler::~EventHandler()
    {
        if (m_source) m_source->unregister_handler(*this);
    }

}

