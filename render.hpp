#pragma once

#include <SDL2/SDL.h>
#include <SDL2pp/Renderer.hh>
#include <SDL2pp/Rect.hh>

#define COL_COUNT  800
#define COL_HEIGHT 500
#define PLAYER_PROJPLANE_DIST 554.0
#define WALLHEIGHT 200.0

#define MAP_SCALE 4
#define MAP_POS_X 0
#define MAP_POS_Y COL_HEIGHT

#define TEXTURE_RES 64

void fillRayAngles();

void render(SDL2pp::Renderer& renderer);