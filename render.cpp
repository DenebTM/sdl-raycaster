#include <cmath>

#include "render.hpp"
#include "globals.hpp"
#include "player.hpp"

#define MAP_SCALE 10
#define MAP_POS_X 0
#define MAP_POS_Y 350

constexpr double FOV = 60 * M_PI / 180;
#define COL_COUNT  640
#define COL_HEIGHT 350
#define PLAYER_PROJPLANE_DIST 554
constexpr double wallProjMult = (double)PLAYER_PROJPLANE_DIST / COL_COUNT;

// angular ray offsets aren't consistent across the entire screen,
// so all angles are pre-calculated
double rayAngles[COL_COUNT] = { 0.0 };
void fillRayAngles() {
    if (rayAngles[0] != 0) return;
    for (int i = 0; i < COL_COUNT; i++) {
        double l = -(COL_COUNT / 2) + 0.5 + i;
        rayAngles[i] = atan(l / PLAYER_PROJPLANE_DIST);
    }
}

void drawPlayer(SDL2pp::Renderer& renderer);
void drawMap(SDL2pp::Renderer& renderer);
double castRay(const double posX, const double posY, double angle);

void render(SDL2pp::Renderer& renderer) {
    renderer.SetDrawColor(0, 0, 0);
    renderer.Clear();

    drawMap(renderer);
    drawPlayer(renderer);

    renderer.SetDrawColor(SDL_Color{255, 0, 0});
    fillRayAngles();
    for (int i = 0; i < COL_COUNT; i++) {
        const double rayAngle = globals::player.angle + rayAngles[i];
        double rayDist = castRay(globals::player.posX, globals::player.posY, rayAngle);
        renderer.DrawLine(MAP_POS_X + globals::player.posX * MAP_SCALE, MAP_POS_Y + globals::player.posY * MAP_SCALE,
            MAP_POS_X + (globals::player.posX + rayDist * sin(rayAngle)) * MAP_SCALE,
            MAP_POS_Y + (globals::player.posY - rayDist * cos(rayAngle)) * MAP_SCALE);
    }

    renderer.Present();
}

void drawPlayer(SDL2pp::Renderer& renderer) {
    using namespace globals;
    const auto oldDrawColor = renderer.GetDrawColor();
    
    renderer.SetDrawColor(255, 255, 0);
    renderer.FillRect(SDL2pp::Rect{
        MAP_POS_X + (int)((player.posX - PLAYER_SIZE / 2) * MAP_SCALE),
        MAP_POS_Y + (int)((player.posY - PLAYER_SIZE / 2) * MAP_SCALE),
        (int)(PLAYER_SIZE * MAP_SCALE), (int)(PLAYER_SIZE * MAP_SCALE)
    });
    renderer.SetDrawColor(oldDrawColor);
}

void drawMap(SDL2pp::Renderer& renderer) {
    using namespace globals;
    const auto oldDrawColor = renderer.GetDrawColor();

    for (int y = 0; y < map.height; y++) {
        for (int x = 0; x < map.width; x++) {
            switch (map.tiles[x][y]) {
                case 0:
                    renderer.SetDrawColor(0, 0, 0);
                    break;
                case 1:
                    renderer.SetDrawColor(127, 127, 127);
                    break;
                default:
                    renderer.SetDrawColor(255, 0, 255);
                    break;
            }

            renderer.FillRect(SDL2pp::Rect{
                MAP_POS_X + MAP_SCALE * x,
                MAP_POS_Y + MAP_SCALE * y,
                MAP_SCALE, MAP_SCALE
            });
        }
    }
    renderer.DrawRect(SDL2pp::Rect{MAP_POS_X, MAP_POS_Y, MAP_SCALE * map.width - 1, MAP_SCALE * map.height - 1});
}

// returns distance to the nearest wall
double castRay(const double posX, const double posY, double angle) {
    while (angle < 0) angle += M_PI * 2;
    angle = fmod(angle, M_PI * 2);

    // find horizontal walls
    double rayDistHoriz = INFINITY;
    if (angle != M_PI_2 && angle != M_PI + M_PI_2) {
        double rayStepY = (angle < M_PI_2 || angle > M_PI + M_PI_2) ? -1 : 1,
               rayStepX = -rayStepY * tan(angle);
        
        double stepInitialY = rayStepY < 0 ? fmod(-posY, 1) : 1 - fmod(posY, 1),
               stepInitialX = rayStepX * stepInitialY / rayStepY;
        double rayPosX = posX + stepInitialX, rayPosY = posY + stepInitialY;

        const int mapPosOffsetY = rayStepY <= 0 ? -1 : 0;

        while ((int)rayPosX >= 0 && (int)rayPosX < globals::map.width
            && (int)rayPosY + mapPosOffsetY >= 0 && (int)rayPosY + mapPosOffsetY < globals::map.height
            && !globals::map.tiles
                [(int)rayPosX]
                [(int)rayPosY + mapPosOffsetY]) {
            rayPosX += rayStepX,
            rayPosY += rayStepY;
        }
        const double rayDistHorizX = rayPosX - posX,
                     rayDistHorizY = rayPosY - posY;
        rayDistHoriz = sqrt(pow(rayDistHorizX, 2) + pow(rayDistHorizY, 2));
        if (rayDistHoriz > 1000) rayDistHoriz = 1000;
    }

    // find vertical walls
    double rayDistVert = INFINITY;
    if (angle != 0 && angle != M_PI) {
        double rayStepX = (angle < M_PI) ? 1 : -1,
               rayStepY = -rayStepX / tan(angle);
        
        double stepInitialX = rayStepX < 0 ? fmod(-posX, 1) : 1 - fmod(posX, 1),
               stepInitialY = rayStepY * stepInitialX / rayStepX;
        double rayPosX = posX + stepInitialX, rayPosY = posY + stepInitialY;

        const int mapPosOffsetX = rayStepX <= 0 ? -1 : 0;

        while ((int)rayPosX + mapPosOffsetX >= 0 && (int)rayPosX + mapPosOffsetX < globals::map.width
            && (int)rayPosY >= 0 && (int)rayPosY < globals::map.height
            && !globals::map.tiles
                [(int)rayPosX + mapPosOffsetX]
                [(int)rayPosY]) {
            rayPosX += rayStepX,
            rayPosY += rayStepY;
        }
        const double rayDistVertX = rayPosX - posX,
                     rayDistVertY = rayPosY - posY;
        rayDistVert = sqrt(pow(rayDistVertX, 2) + pow(rayDistVertY, 2));
        if (rayDistVert > 1000) rayDistVert = 1000;
    }
    
    return (rayDistHoriz < rayDistVert) ? rayDistHoriz : rayDistVert;
}