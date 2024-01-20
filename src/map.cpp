#include "map.hpp"

#include <fstream>

Map::Map(int w, int h) {
    init(w, h);
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            if (x == 0 || x == width-1 || y == 0 || y == height-1 || (y == (height/2) && x < (width/2)))
                tiles[x][y] = 1;
        }
    }
}

void Map::init(int w, int h) {
    width = w;
    height = h;

    tiles = new char*[width];
    for (int x = 0; x < width; x++)
        tiles[x] = new char[height];
}

Map::Map(std::string filename) {
    readMap(filename);
}

void Map::readMap(std::string filename) {
    std::ifstream file(filename);
    file >> width >> height >> playerStartX >> playerStartY;

    init(width, height);
    for (int l = 0; l < height; l++) {
        std::string line;
        file >> line;
        parseLine(line, l);
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            printf("%2d", tiles[x][y]);
        }
        puts("");
    }
}

void Map::parseLine(std::string line, int y) {
    for (int x = 0; x < width; x++) {
        int hex = line.at(x);
        hex = hex >= 'a' ? 10 + hex - 'a' : hex - '0';
        tiles[x][y] = hex;
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
