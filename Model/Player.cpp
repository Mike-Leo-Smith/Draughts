#include <random>
#include "Player.h"

std::string PlayerHelper::toString(Player player)
{
    if (player == Player::black) {
        return "black";
    }
    if (player == Player::white) {
        return "white";
    }
    return "empty";
}

Player PlayerHelper::fromString(const std::string &player)
{
    if (player == "black") {
        return Player::black;
    }
    if (player == "white") {
        return Player::white;
    }
    return Player::empty;
}

Player PlayerHelper::random()
{
    if (rand() % 2 == 0) {
        return Player::black;
    } else {
        return Player::white;
    }
}

Player PlayerHelper::opponent(Player player)
{
    if (player == Player::black) {
        return Player::white;
    }
    if (player == Player::white) {
        return Player::black;
    }
    return Player::empty;
}
