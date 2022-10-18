#pragma once

#include "mapObject.hpp"

#define PLAYER_VELOCITY_BASE 5.0
#define PLAYER_ANGVEL_BASE 2.5
#define PLAYER_SIZE 0.5

class Player: public MapObject {
    public:
        double posX, posY, angle;
        double fVel, sVel, aVel;
        
        Player();
        Player(double x, double y);
        void doTick();

    private:

};