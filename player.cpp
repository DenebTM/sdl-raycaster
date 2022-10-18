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
    if (fVel == 0 && sVel == 0 && aVel == 0) return;

    double velX = (fVel * sin(angle) + sVel * cos(angle)) * velMult,
           velY = (-fVel * cos(angle) + sVel * sin(angle)) * velMult;

    double newPosX = posX + velX * globals::deltaTime,
           newPosY = posY + velY * globals::deltaTime;
    
    while (globals::map.checkCollision(newPosX, posY, PLAYER_SIZE)) {
        newPosX -= (velX * globals::deltaTime) / 10;
    }
    posX = newPosX;
    while (globals::map.checkCollision(posX, newPosY, PLAYER_SIZE)) {
        newPosY -= (velY * globals::deltaTime) / 10;
    }
    posY = newPosY;

    if (aVel >= 0.01 || aVel <= -0.01)
        angle = fmod(angle + M_PI * 2 + aVel * velMult * globals::deltaTime, M_PI * 2);
}