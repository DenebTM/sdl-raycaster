#include "map.hpp"

Map::Map(int w, int h) {
    width = w;
    height = h;

    tiles = new char*[width];
    for (int x = 0; x < width; x++) {
        tiles[x] = new char[height];

        for (int y = 0; y < height; y++) {
            if (x == 0 || x == width-1 || y == 0 || y == height-1 || (y == (height/2) && x < (width/2)))
                tiles[x][y] = 1;
        }
    }
}

Map::~Map() {
    if (tiles) {
        for (int x = 0; x < width; x++) {
            if (tiles[x])
                delete[] tiles[x];
        }

        delete[] tiles;
    }
}

bool Map::checkCollision(double posX, double posY, double radius) {
    int posXLeft   = (int)(posX - (radius/2)),
        posXRight  = (int)(posX + (radius/2)),
        posYTop    = (int)(posY - (radius/2)),
        posYBottom = (int)(posY + (radius/2));

    return (posXLeft < 0 || posXRight >= width || posYTop < 0 || posYBottom >= height ||
        tiles[posXLeft][posYTop] || tiles[posXLeft][posYBottom] ||
        tiles[posXRight][posYTop] || tiles[posXRight][posYBottom]);
}