
#include <iostream>
#include <SDL2/SDL.h>

#include "context.hpp"
#include "window.hpp"

int main(int argc, char* argv[])
{
    std::cout << "<start>" << std::endl;

    auto context = sol::create_context();
    auto& ctx = *context;

    auto wh = ctx.windows().create(800,600);

    while (!ctx.events().should_quit()) {
        ctx.events().update();
    }

    std::cout << "<end>" << std::endl;

    char c;
    std::cin >> c;
    return 0;
}
