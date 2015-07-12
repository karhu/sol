
#include <iostream>
#include <SDL2/SDL.h>

#include "context.hpp"
#include "window.hpp"

int main(int argc, char* argv[])
{
    std::cout << "<start>" << std::endl;

    auto context = sol::create_context();
    auto win = context->create_window(800,600);
    auto win2 = context->create_window(1280,720);

    auto wh = context->windows().create(800,600);

    while (!context->events().should_quit()) {
        context->events().update();
    }

    std::cout << "<end>" << std::endl;

    char c;
    std::cin >> c;
    return 0;
}
