#pragma once

#define COL_COUNT  800
#define COL_HEIGHT 500
#define PLAYER_PROJPLANE_DIST 554.0
#define WALLHEIGHT 200.0

#define MAP_SCALE 4
#define MAP_POS_X 0
#define MAP_POS_Y COL_HEIGHT

#define TEXTURE_RES 64

namespace GameRenderer {
    void init(SDL2pp::Renderer& renderer);
    void render();
}