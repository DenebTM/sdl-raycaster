#pragma once

#define PLAYER_VELOCITY_BASE 15.0
#define PLAYER_SIZE 0.5

class Player {
    public:
        double posX, posY, angle;
        double velX, velY, velA;
        
        Player();
        Player(double x, double y);
        void doTick();

    private:

};