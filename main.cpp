#include <iostream>
#include <signal.h>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL.hh>
#include <SDL2pp/Window.hh>
#include <SDL2pp/Renderer.hh>

#include "globals.hpp"
#include "events.hpp"

using namespace SDL2pp;

void sigh(int signum) { globals::stop = 1; }

int main(int, char**) {
    signal(SIGINT, &sigh);

    // initialize SDL, telling it not to inhibit desktop composition
    SDL sdl(SDL_INIT_VIDEO);
    SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");

    Window window("SDL Test",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        800, 600,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    Renderer renderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    //main loop
    while (!globals::stop) {
        handle_events();

        // render
        renderer.SetDrawColor(0, 0, 0);
        renderer.Clear();
        renderer.SetDrawColor(255, 255, 0);
        renderer.FillRect(120, 90, 520, 390);
        renderer.Present();

        // frame limit (handled by vsync now)
        // SDL_Delay(1);
    }

    return EXIT_SUCCESS;
}
