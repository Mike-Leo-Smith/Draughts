//
// Created by Mike Smith on 05/09/2017.
//

#ifndef DRAUGHTS_PLAYER_H
#define DRAUGHTS_PLAYER_H

#include <string>

enum class Player {
    black, white, empty
};

struct PlayerHelper {
    static std::string toString(Player player);
    static Player fromString(const std::string &player);
    static Player random();
    static Player opponent(Player player);
    PlayerHelper() = delete;
};

#endif  // DRAUGHTS_PLAYER_H
