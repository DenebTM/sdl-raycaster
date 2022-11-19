#pragma once
#include "player.hpp"
#include "map.hpp"

namespace globals {
    inline bool stop = false;
    inline double deltaTime = 0.0;

    inline Map map("maps/map01.txt");
    inline Player player(map.playerStartX + 0.5, map.playerStartY + 0.5);
}
