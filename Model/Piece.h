//
// Created by Mike Smith on 05/09/2017.
//

#ifndef DRAUGHTS_PIECE_H
#define DRAUGHTS_PIECE_H

#include "Player.h"

enum class PieceType {
    normal, crowned
};

using PieceColor = Player;

struct Piece {
    PieceColor color;
    PieceType type;
};

#endif  // DRAUGHTS_PIECE_H
