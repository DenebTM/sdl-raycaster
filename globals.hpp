#pragma once
#include "player.hpp"
#include "map.hpp"

namespace globals {
    inline bool stop = false;
    inline double deltaTime = 0.0;

    inline Map map = Map(25, 25);
    inline Player player = Player(12.5, 12.5);
}