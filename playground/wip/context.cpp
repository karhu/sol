#include "context.hpp"

#include <SDL2/SDL.h>

using namespace arc;

namespace sol 
{

uint32_t Context::s_counter = 0;

Context::Context()
    : m_events(*this)
    , m_window_system(*this)
{
    if (Context::s_counter == 0)
    {
        // Enable standard application logging
        SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

        // init SDL
        if (SDL_Init(SDL_INIT_VIDEO) < 0)
        {
            // TODO: log error
            //SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s\n", SDL_GetError());
            m_valid = false;
        }
    }

    Context::s_counter += 1;
}

Context::~Context()
{
    Context::s_counter -= 1;

    if (Context::s_counter == 0)
    {
        SDL_Quit();
    }
}

// Context construction //

class ContextT : public Context {
public:
    ContextT() : Context() {}
};

uptr<Context> get_context()
{
    uptr<Context> context = std::make_unique<ContextT>();
    if (context->m_valid) {
        return context;
    } else {
        return nullptr;
    }
}

}
