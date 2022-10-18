#include <iostream>
#include <signal.h>
#include <chrono>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL.hh>
#include <SDL2pp/Window.hh>
#include <SDL2pp/Renderer.hh>

#include "globals.hpp"
#include "events.hpp"
#include "player.hpp"
#include "render.hpp"

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
    GameRenderer::init(renderer);

    auto t1 = std::chrono::system_clock::now(), t2 = t1;

    //main loop
    while (!globals::stop) {
        // keep track of time to keep the game speed constant
        { using namespace std::chrono;
            t2 = system_clock::now();
            globals::deltaTime = (double)duration_cast<milliseconds>(t2 - t1).count() / 1000;
            t1 = t2;
        }

        handle_events();
        
        globals::player.doTick();

        GameRenderer::render(renderer);
    }

    return EXIT_SUCCESS;
}
