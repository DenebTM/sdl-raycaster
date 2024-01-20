#pragma once

#include <SDL2/SDL.h>
#include <SDL2pp/Renderer.hh>

#define MAP_SCALE 4
#define MAP_POS_X 0
#define MAP_POS_Y colHeight

#define TEXTURE_RES 64
#ifndef RENDER_THEAD_COUNT
#define RENDER_THREAD_COUNT 1
#endif

namespace GameRenderer {
    void init(SDL2pp::Renderer& renderer);
    void destroy();
    void render();
    void resize();
}
