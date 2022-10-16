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
                if (event.key.repeat > 0) break;
                std::cout << "Key pressed:    " << SDL_GetKeyName(event.key.keysym.sym) << "\n";

                switch (event.key.keysym.scancode) {
                    case SDL_SCANCODE_W:
                        globals::player.fVel += PLAYER_VELOCITY_BASE;
                        break;
                    case SDL_SCANCODE_A:
                        globals::player.sVel -= PLAYER_VELOCITY_BASE;
                        break;
                    case SDL_SCANCODE_S:
                        globals::player.fVel -= PLAYER_VELOCITY_BASE;
                        break;
                    case SDL_SCANCODE_D:
                        globals::player.sVel += PLAYER_VELOCITY_BASE;
                        break;
                    case SDL_SCANCODE_LEFT:
                        globals::player.aVel -= PLAYER_ANGVEL_BASE;
                        break;
                    case SDL_SCANCODE_RIGHT:
                        globals::player.aVel += PLAYER_ANGVEL_BASE;
                        break;

                    default:
                        break;
                }
                break;
            case SDL_KEYUP:
                std::cout << "Key released:   " << SDL_GetKeyName(event.key.keysym.sym) << "\n";
                switch (event.key.keysym.scancode) {
                    case SDL_SCANCODE_W:
                        globals::player.fVel -= PLAYER_VELOCITY_BASE;
                        break;
                    case SDL_SCANCODE_A:
                        globals::player.sVel += PLAYER_VELOCITY_BASE;
                        break;
                    case SDL_SCANCODE_S:
                        globals::player.fVel += PLAYER_VELOCITY_BASE;
                        break;
                    case SDL_SCANCODE_D:
                        globals::player.sVel -= PLAYER_VELOCITY_BASE;
                        break;
                    case SDL_SCANCODE_LEFT:
                        globals::player.aVel += PLAYER_ANGVEL_BASE;
                        break;
                    case SDL_SCANCODE_RIGHT:
                        globals::player.aVel -= PLAYER_ANGVEL_BASE;
                        break;

                    default:
                        break;
                }
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