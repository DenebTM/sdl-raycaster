#include <cmath>
#include <vector>
#include <filesystem>
#include <string>
#include <SDL2/SDL.h>
#include <SDL2pp/Renderer.hh>
#include <SDL2pp/Texture.hh>

#include "render.hpp"
#include "globals.hpp"
#include "player.hpp"

namespace GameRenderer {
    enum wall { N, S, E, W };
    typedef struct ray_ret {
        double rayDist;
        wall wallDir;
        char texturePos;
    } ray;

    SDL2pp::Renderer* mainRenderer;
    
    void fillRayAngles();
    void loadTextures();
    void drawStatusBar();
    void drawPlayer();
    void drawMap();
    void drawColumn(int x, int height, char texturePos, char texture);
    void drawScreen();
    ray castRay(const double posX, const double posY, double angle);

    void init(SDL2pp::Renderer& renderer) {
        mainRenderer = &renderer;

        fillRayAngles();
        loadTextures();
    }

    // angular ray offsets aren't consistent across the entire screen,
    // so all angles are pre-calculated
    double rayAngles[COL_COUNT] = { 0.0 };
    void fillRayAngles() {
        for (int i = 0; i < COL_COUNT; i++) {
            double l = -(COL_COUNT / 2) + 0.5 + i;
            rayAngles[i] = atan(l / PLAYER_PROJPLANE_DIST);
        }
    }

    std::vector<std::string> texPaths;
    std::vector<SDL2pp::Texture> textures;
    void loadTextures() {
        namespace fs = std::filesystem;
        std::string texBasePath = "./textures";

        for (const auto entry : fs::directory_iterator(texBasePath))
            if (entry.is_regular_file())
                texPaths.push_back(entry.path().string());

        std::sort(texPaths.begin(), texPaths.end());

        for (const auto path : texPaths)
            textures.push_back(SDL2pp::Texture{*mainRenderer, path});
    }

    void render() {
        mainRenderer->SetDrawColor(0, 0, 0);
        mainRenderer->Clear();

        drawScreen();
        drawStatusBar();

        mainRenderer->Present();
    }

    void drawStatusBar() {
        mainRenderer->SetDrawColor(SDL_Color{0, 0, 0});
        mainRenderer->FillRect(SDL_Rect{0, COL_HEIGHT, COL_COUNT, 100});
        drawMap();
        drawPlayer();
    }

    void drawPlayer() {
        using namespace globals;
        const auto oldDrawColor = mainRenderer->GetDrawColor();
        
        mainRenderer->SetDrawColor(255, 255, 0);
        mainRenderer->FillRect(SDL2pp::Rect{
            MAP_POS_X + (int)((player.posX - PLAYER_SIZE / 2) * MAP_SCALE),
            MAP_POS_Y + (int)((player.posY - PLAYER_SIZE / 2) * MAP_SCALE),
            (int)(PLAYER_SIZE * MAP_SCALE), (int)(PLAYER_SIZE * MAP_SCALE)
        });
        mainRenderer->SetDrawColor(oldDrawColor);
    }

    void drawMap() {
        using namespace globals;
        const auto oldDrawColor = mainRenderer->GetDrawColor();

        for (int y = 0; y < map.height; y++) {
            for (int x = 0; x < map.width; x++) {
                switch (map.tiles[x][y]) {
                    case 0:
                        mainRenderer->SetDrawColor(0, 0, 0);
                        break;
                    case 1:
                        mainRenderer->SetDrawColor(127, 127, 127);
                        break;
                    default:
                        mainRenderer->SetDrawColor(255, 0, 255);
                        break;
                }

                mainRenderer->FillRect(SDL2pp::Rect{
                    MAP_POS_X + MAP_SCALE * x,
                    MAP_POS_Y + MAP_SCALE * y,
                    MAP_SCALE, MAP_SCALE
                });
            }
        }
        mainRenderer->DrawRect(SDL2pp::Rect{MAP_POS_X, MAP_POS_Y, MAP_SCALE * map.width - 1, MAP_SCALE * map.height - 1});
        mainRenderer->SetDrawColor(oldDrawColor);
    }

    void drawColumn(int x, int h, char texturePos, char textureId) {
        // if (h > COL_HEIGHT) h = COL_HEIGHT;
        mainRenderer->Copy(textures.at(textureId), SDL_Rect{texturePos, 0, 1, 64}, SDL_Rect{x, (COL_HEIGHT - h) / 2, 1, h});
    }

    void drawScreen() {
        mainRenderer->SetDrawColor(SDL_Color{20, 20, 20});
        mainRenderer->FillRect(SDL_Rect{0, 0, COL_COUNT, COL_HEIGHT/2});
        mainRenderer->SetDrawColor(SDL_Color{40, 40, 40});
        mainRenderer->FillRect(SDL_Rect{0, COL_HEIGHT/2, COL_COUNT, COL_HEIGHT/2});

        for (int x = 0; x < COL_COUNT; x++) {
            using namespace globals;
            const double rayAngle = player.angle + rayAngles[x];
            ray r = castRay(player.posX, player.posY, rayAngle);
            int wallHeight = (WALLHEIGHT / r.rayDist) * 2.7754 / cos(rayAngles[x]);
            drawColumn(x, wallHeight, r.texturePos, 0 + (r.wallDir >= E));
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

        double rDist;
        wall wDir;
        char tPos;
        if (rayDistHoriz < rayDistVert) {
            rDist = rayDistHoriz;
            wDir = rayStepY < 0 ? N : S;
            tPos = fmod(rayPosFinalX, 1) * TEXTURE_RES;
            if (wDir == S) tPos = TEXTURE_RES - tPos - 1;
        } else {
            rDist = rayDistVert;
            wDir = rayStepX < 0 ? W : E;
            tPos = fmod(rayPosFinalY, 1) * TEXTURE_RES;
            if (wDir == W) tPos = TEXTURE_RES - tPos - 1;
        }
        
        return {
            .rayDist = rDist,
            .wallDir = wDir,
            .texturePos = tPos
        };
    }
}