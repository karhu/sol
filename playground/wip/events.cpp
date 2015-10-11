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
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            //std::cout << "e: " << e.type << std::endl;
            switch (e.type) {
                case SDL_QUIT:
                    m_should_quit = true;
                    break;
                case SDL_WINDOWEVENT: {
                    auto ew = e.window;
                    //std::cout << "ew: " << ew.type << std::endl;
                    switch (ew.event) {
                        case SDL_WINDOWEVENT_CLOSE:
                            m_context.m_window_system.ev_close_request(ew.windowID);
                            break;

                    }
                }
            }
        }
    }

    void wait()
    {
        // TODO
    }
}

