#include <iostream>
#include <SDL2/SDL.h>
#include "globals.hpp"
#include "events.hpp"

int handle_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_WINDOWEVENT:
                switch (event.window.event) {
                    case SDL_WINDOWEVENT_CLOSE:
                        globals::stop = 1;
                        break;
                    case SDL_WINDOWEVENT_FOCUS_GAINED:
                        std::cout << "Window focused\n";
                        break;
                    case SDL_WINDOWEVENT_FOCUS_LOST:
                        std::cout << "Window unfocused\n";
                        break;
                    default:
                        break;
                }
                break;
            case SDL_KEYDOWN:
                std::cout << "Key pressed:    " << SDL_GetKeyName(event.key.keysym.sym) << "\n";
                break;
            case SDL_KEYUP:
                std::cout << "Key released:   " << SDL_GetKeyName(event.key.keysym.sym) << "\n";
                break;
            case SDL_MOUSEMOTION:
                std::cout << "Mouse position: x=" << event.motion.x << ", y=" << event.motion.y << "\n";
                break;
            default:
                break;
        }
    }

    return 0;
}