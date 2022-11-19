#pragma once

#include <string>

class Map {
    public:
        int width, height;
        int playerStartX, playerStartY;
        char** tiles;

        Map(int w, int h);
        Map(std::string filename);
        ~Map();

        bool checkCollision(double posX, double posY, double radius);
    
    private:
        void init(int w, int h);
        void parseLine(std::string line, int y);
        void readMap(std::string filename);
};