
#include <iostream>
#include <SDL2/SDL.h>

#include "context.hpp"
#include "window.hpp"

int main(int argc, char* argv[])
{
    std::cout << "<start>" << std::endl;

    auto context = sol::get_context();
    auto& ctx = *context;

    auto wh = ctx.windows().create(800,600);

    vec2f origin = {0.5,1.0};
    vec2f dimension = {0.5,-0.5};

    Transform2f t;
    t.scale = dimension;
    t.translation = origin;
    auto it = inverse(t);

    while (!ctx.events().should_quit()) {
        ctx.events().update();
    }

    std::cout << "<end>" << std::endl;

    return 0;
}
