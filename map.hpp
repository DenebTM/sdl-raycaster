#pragma once

class Map {
    public:
        int width, height;
        char** tiles;

        Map(int w, int h);
        ~Map();

        bool checkCollision(double posX, double posY, double radius);
};