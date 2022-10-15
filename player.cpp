#include "globals.hpp"
#include "player.hpp"
#include <iostream>
#include <math.h>

Player::Player() {
    Player(0, 0);
}

Player::Player(double x, double y) {
    posX = x;
    posY = y;
    angle = 0;
}

void Player::doTick() {
    if (velX == 0 && velY == 0 && velA == 0) return;

    double newPosX = posX + velX * globals::deltaTime,
           newPosY = posY + velY * globals::deltaTime;
    
    if (!globals::map.checkCollision(newPosX, posY, PLAYER_SIZE))
        posX = newPosX;
    if (!globals::map.checkCollision(posX, newPosY, PLAYER_SIZE))
        posY = newPosY;

    if (velA >= 0.01 || velA <= -0.01)
        angle = fmod(angle + M_PI * 2 + velA * globals::deltaTime, M_PI * 2);
}