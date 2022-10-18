#include <cmath>

#include "render.hpp"
#include "globals.hpp"
#include "player.hpp"

// angular ray offsets aren't consistent across the entire screen,
// so all angles are pre-calculated
double rayAngles[COL_COUNT] = { 0.0 };
void fillRayAngles() {
    for (int i = 0; i < COL_COUNT; i++) {
        double l = -(COL_COUNT / 2) + 0.5 + i;
        rayAngles[i] = atan(l / PLAYER_PROJPLANE_DIST);
    }
}

enum wall { N, E, S, W };
typedef struct ray_ret {
    double rayDist;
    wall wallDir;
    int texturePos;
} ray;

void drawPlayer(SDL2pp::Renderer& renderer);
void drawMap(SDL2pp::Renderer& renderer);
void drawScreen(SDL2pp::Renderer& renderer);
ray castRay(const double posX, const double posY, double angle);

void render(SDL2pp::Renderer& renderer) {
    renderer.SetDrawColor(0, 0, 0);
    renderer.Clear();

    drawMap(renderer);
    drawPlayer(renderer);
    drawScreen(renderer);

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

void drawScreen(SDL2pp::Renderer& renderer) {
    renderer.SetDrawColor(SDL_Color{20, 20, 20});
    renderer.FillRect(SDL_Rect{0, 0, COL_COUNT, COL_HEIGHT/2});
    renderer.SetDrawColor(SDL_Color{40, 40, 40});
    renderer.FillRect(SDL_Rect{0, COL_HEIGHT/2, COL_COUNT, COL_HEIGHT/2});

    for (int i = 0; i < COL_COUNT; i++) {
        using namespace globals;
        const double rayAngle = player.angle + rayAngles[i];
        ray r = castRay(player.posX, player.posY, rayAngle);
    
        renderer.SetDrawColor(SDL_Color{255, 0, 0});
        renderer.DrawLine(MAP_POS_X + player.posX * MAP_SCALE, MAP_POS_Y + player.posY * MAP_SCALE,
            MAP_POS_X + (player.posX + r.rayDist * sin(rayAngle)) * MAP_SCALE,
            MAP_POS_Y + (player.posY - r.rayDist * cos(rayAngle)) * MAP_SCALE);

        int wallHeight = (WALLHEIGHT / r.rayDist) * 2 / cos(rayAngles[i]);
        switch (r.wallDir) {
            case N: case S:
                renderer.SetDrawColor(SDL_Color{80, 80, 80});
                break;
            default: 
                renderer.SetDrawColor(SDL_Color{120, 120, 120});
        }
        if (wallHeight > COL_HEIGHT) wallHeight = COL_HEIGHT;
        renderer.DrawLine(i, (COL_HEIGHT - wallHeight) / 2, i, (COL_HEIGHT + wallHeight) / 2);
    }
}

// returns distance to the nearest wall
ray castRay(const double posX, const double posY, double angle) {
    while (angle < 0) angle += M_PI * 2;
    angle = fmod(angle, M_PI * 2);

    double rayStepX, rayStepY,
           rayPosFinalX, rayPosFinalY;

    // find horizontal walls
    double rayDistHoriz = INFINITY;
    if (angle != M_PI_2 && angle != M_PI + M_PI_2) {
        rayStepY = (angle < M_PI_2 || angle > M_PI + M_PI_2) ? -1 : 1;
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
        rayPosFinalX = rayPosX;
    }

    // find vertical walls
    double rayDistVert = INFINITY;
    if (angle != 0 && angle != M_PI) {
        rayStepX = (angle < M_PI) ? 1 : -1;
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
        rayPosFinalY = rayPosY;
    }

    double rayDist;
    wall dir;
    int pos;
    if (rayDistHoriz < rayDistVert) {
        rayDist = rayDistHoriz;
        dir = rayStepY < 0 ? N : S;
        pos = fmod(rayPosFinalX, 1) * TEXTURE_RES;
        if (dir == S) pos = TEXTURE_RES - pos;
    } else {
        rayDist = rayDistVert;
        dir = rayStepX < 0 ? W : E;
        pos = fmod(rayPosFinalY, 1) * TEXTURE_RES;
        if (dir == W) pos = TEXTURE_RES - pos;
    }
    
    return {
        .rayDist = rayDist,
        .wallDir = dir,
        .texturePos = pos
    };
}