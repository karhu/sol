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
        vec2f origin = {0.5,1.0};
        vec2f dimension = {0.5,-0.5};

        Transform2f t;
        t.scale = dimension;
        t.translation = origin;
        auto it = inverse(t);

        SDL_Event e;
        while (SDL_PollEvent(&e)) {
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
                break;
                case SDL_FINGERDOWN: {
                    auto ew = e.tfinger;

                    vec2f p{ew.x,ew.y};
                    auto pt  = transform_point(t,p);
                    auto pit = transform_point(it,p);

                    std::cout << ew.x << ", "
                              << ew.y << " / "
                              << pt.x << ", "
                              << pt.y << " / "
                              << pit.x <<  ", "
                              << pit.y << std::endl;
                    break;
                }
            }
        }
    }

    void Events::emit_should_quit()
    {
        m_should_quit = true;
    }

    void wait()
    {
        // TODO
    }
}

