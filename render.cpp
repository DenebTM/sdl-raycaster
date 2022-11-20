#include <cmath>
#include <vector>
#include <filesystem>
#include <string>
#include <iostream>
#include <pthread.h>

#include <SDL2pp/Texture.hh>
#include <SDL2pp/Surface.hh>
#include <SDL2pp/Point.hh>

#include "render.hpp"
#include "globals.hpp"
#include "player.hpp"

namespace GameRenderer {
    enum wall { N, S, E, W };
    typedef struct ray_ret {
        double rayDist;
        wall wallDir;
        uint8_t textureId;
        uint8_t texturePos;
    } ray;
    typedef struct floor_ray_ret {
        double intersectX;
        double intersectY;
    } floor_ray;

    SDL2pp::Renderer* mainRenderer;
    bool resized = false, firstRun = true;

    // default values, will be overwritten by init
    int colCount = 800;
    int colHeight = 500;
    double FOV = M_PI / 3 /* 60° */,
           projplaneDist = (colCount / 2) / tan(FOV/2);

    Uint32* floorPixels = new Uint32[colCount * colHeight / 2];
    SDL2pp::Surface floorSurf = SDL2pp::Surface{floorPixels, colCount, colHeight/2, 32, 4*colCount, 0x000000ff, 0x0000ff00, 0x00ff0000, 0};
    
    std::vector<double> rayAngles;
    std::vector<double> rayAnglesVert;
    void fillRayAngles();
    void loadTextures();
    void drawStatusBar();
    void drawPlayer();
    void drawMap();
    void drawFloor();
    void drawWall(int x, int height, char texturePos, char texture);
    void drawScreen();
    ray castRay(const double posX, const double posY, double angle);
    floor_ray castFloorRay(const double posX, const double posY, double angleH, double angleV);

    #define RENDER_THREAD_COUNT 8
    bool shall_exit = false;
    pthread_t renderThreads[RENDER_THREAD_COUNT];
    pthread_barrier_t renderStart;
    pthread_barrier_t renderDone;
    void drawFloorPart(uintptr_t threadnum);
    void* renderWorker(void* threadnum) {
        while (!shall_exit) {
            // wait until there is work to do
            pthread_barrier_wait(&renderStart);

            drawFloorPart((uintptr_t)threadnum);

            // signal that work has been done
            pthread_barrier_wait(&renderDone);
        }

        return NULL;
    }

    void init(SDL2pp::Renderer& renderer) {
        if (firstRun) {
            mainRenderer = &renderer;
            loadTextures();
        }

        SDL2pp::Point p = mainRenderer->GetOutputSize();
        colCount = p.GetX();
        colHeight = p.GetY() - 100;
        FOV = (double)colCount / colHeight * 0.655;
        projplaneDist = (colCount / 2) / tan(FOV/2);
        fillRayAngles();

        delete floorPixels;
        floorPixels = new Uint32[colCount * colHeight / 2];
        floorSurf = SDL2pp::Surface{floorPixels, colCount, colHeight/2, 32, 4*colCount, 0x000000ff, 0x0000ff00, 0x00ff0000, 0};

        if (firstRun) {
            pthread_barrierattr_t ptba;
            pthread_barrierattr_init(&ptba);
            pthread_barrier_init(&renderStart, &ptba, RENDER_THREAD_COUNT+1);
            pthread_barrier_init(&renderDone, &ptba, RENDER_THREAD_COUNT+1);
            pthread_attr_t pta;
            pthread_attr_init(&pta);
            for (uintptr_t i = 0; i < RENDER_THREAD_COUNT; i++)
                pthread_create(&renderThreads[i], &pta, &renderWorker, (void*)i);

            firstRun = false;
        }
    }

    void destroy() {
        shall_exit = true;
        pthread_barrier_wait(&renderStart);
        pthread_barrier_wait(&renderDone);
        delete floorPixels;
    }

    // the angles between each scanlines/columns aren't consistent, so these are pre-calculated
    void fillRayAngles() {
        rayAngles.clear();
        rayAngles.reserve(colCount);
        for (int i = 0; i < colCount; i++) {
            double l = -(colCount / 2) + 0.5 + i;
            rayAngles[i] = atan(l / projplaneDist);
        }

        rayAnglesVert.clear();
        rayAnglesVert.reserve(colHeight/2);
        for (int i = 0; i < colHeight/2; i++) {
            double h = 0.5 + i;
            rayAnglesVert[i] = atan(h / projplaneDist);
        }
    }

    std::vector<std::string> texPaths;
    std::vector<SDL2pp::Texture> textures;
    std::vector<SDL2pp::Surface> texturesPix;
    void loadTextures() {
        namespace fs = std::filesystem;
        std::string texBasePath = "./textures";

        for (const auto entry : fs::directory_iterator(texBasePath))
            if (entry.is_regular_file())
                texPaths.push_back(entry.path().string());

        std::sort(texPaths.begin(), texPaths.end());

        for (const auto path : texPaths) {
            textures.push_back(SDL2pp::Texture{*mainRenderer, path});
            texturesPix.push_back(SDL2pp::Surface{path});
        }
    }

    void resize() { resized = true; }

    void render() {
        if (resized) {
            init(*mainRenderer);
            resized = false;
        }

        mainRenderer->SetDrawColor(0, 0, 0);
        mainRenderer->Clear();

        drawScreen();
        drawStatusBar();

        mainRenderer->Present();
    }

    void drawStatusBar() {
        mainRenderer->SetDrawColor(SDL_Color{0, 0, 0});
        mainRenderer->FillRect(SDL_Rect{0, colHeight, colCount, 100});
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

    void drawFloorPart(uintptr_t threadnum) {
        using namespace globals;
        Uint32* floorTexturePixels = (Uint32*)texturesPix[14].Get()->pixels;

        const int surfHeight = floorSurf.GetHeight();
        const int startLine = (surfHeight / RENDER_THREAD_COUNT) * threadnum,
                  endLine = startLine + (surfHeight / RENDER_THREAD_COUNT);
        for (int line = startLine; line < endLine; line++) {
            // cast two rays for the left- and rightmost pixels
            const floor_ray r1 = castFloorRay(player.posX, player.posY,
                player.angle + rayAngles[0], rayAnglesVert[line]),
            r2 = castFloorRay(player.posX, player.posY,
                player.angle + rayAngles[colCount-1], rayAnglesVert[line]);

            // calculate the map distance between each pixel
            const double stepX = (r2.intersectX - r1.intersectX) / colCount,
                         stepY = (r2.intersectY - r1.intersectY) / colCount;

            // fill the entire line
            double floorPosX = r1.intersectX, floorPosY = r1.intersectY;
            for (int col = 0; col < colCount; col++) {
                uint8_t textureX = fmod(floorPosX, 1) * 64,
                        textureY = fmod(floorPosY, 1) * 64;
                if (textureX < 0) textureX += 64;
                if (textureY < 0) textureY += 64;
                
                // TODO: account for different pixel formats and texture sizes
                Uint32 color = floorTexturePixels[64*textureY + textureX];
                floorPixels[line*colCount + col] = color;
                floorPosX += stepX, floorPosY += stepY;
            }
        }
    }

    void drawFloor() {
        using namespace globals;
        Uint32* floorTexturePixels = (Uint32*)texturesPix[14].Get()->pixels;

        const int surfHeight = floorSurf.GetHeight();
        pthread_barrier_wait(&renderStart);
        pthread_barrier_wait(&renderDone);

        SDL2pp::Texture floorTex(*mainRenderer, floorSurf);
        mainRenderer->Copy(floorTex, SDL2pp::NullOpt, SDL_Point{0, surfHeight});
    }

    void drawWall(int x, int h, char texturePos, char textureId) {
        mainRenderer->Copy(textures[textureId], SDL_Rect{texturePos, 0, 1, 64}, SDL_Rect{x, (colHeight - h) / 2, 1, h});
    }

    void drawScreen() {
        mainRenderer->SetDrawColor(SDL_Color{20, 20, 20});
        mainRenderer->FillRect(SDL_Rect{0, 0, colCount, colHeight/2});
        mainRenderer->SetDrawColor(SDL_Color{40, 40, 40});
        mainRenderer->FillRect(SDL_Rect{0, colHeight/2, colCount, colHeight/2});

        drawFloor();
        for (int x = 0; x < colCount; x++) {
            using namespace globals;
            const double rayAngle = player.angle + rayAngles[x];
            ray r = castRay(player.posX, player.posY, rayAngle);
            int wallHeight = projplaneDist / r.rayDist / cos(rayAngles[x]);
            drawWall(x, wallHeight, r.texturePos, r.textureId + (r.wallDir >= E));
        }
    }

    // returns distance to the nearest wall
    ray castRay(const double posX, const double posY, double angle) {
        while (angle < 0) angle += M_PI * 2;
        angle = fmod(angle, M_PI * 2);

        double rayStepX, rayStepY,
               rayPosFinalX1, rayPosFinalY1,
               rayPosFinalX2, rayPosFinalY2;

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
            rayPosFinalX1 = rayPosX;
            rayPosFinalY1 = rayPosY;
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
            rayPosFinalX2 = rayPosX;
            rayPosFinalY2 = rayPosY;
        }

        double rDist;
        wall wDir;
        uint8_t tId = 0;
        uint8_t tPos;
        if (rayDistHoriz < rayDistVert) {
            rDist = rayDistHoriz;
            wDir = rayStepY < 0 ? N : S;
            tId = globals::map.tiles[(int)rayPosFinalX1][(int)rayPosFinalY1 - ((angle < M_PI_2 || angle >= M_PI + M_PI_2) ? 1 : 0)];
            tPos = fmod(rayPosFinalX1, 1) * TEXTURE_RES;
            if (wDir == S) tPos = TEXTURE_RES - tPos - 1;
        } else {
            rDist = rayDistVert;
            wDir = rayStepX < 0 ? W : E;
            tId = globals::map.tiles[(int)rayPosFinalX2 - (angle >= M_PI ? 1 : 0)][(int)rayPosFinalY2];
            tPos = fmod(rayPosFinalY2, 1) * TEXTURE_RES;
            if (wDir == W) tPos = TEXTURE_RES - tPos - 1;
        }
        
        return {
            .rayDist = rDist,
            .wallDir = wDir,
            .textureId = (unsigned char)((tId-1)*2),
            .texturePos = tPos
        };
    }

    floor_ray castFloorRay(const double posX, const double posY, double angleH, double angleV) {
        using namespace globals;

        double d  = (0.5 / tan(angleV)) / cos(angleH - player.angle);
        double dx = d * sin(angleH),
               dy = d * cos(angleH);
        
        return {
            .intersectX = posX + dx,
            .intersectY = posY - dy
        };
    }
}
