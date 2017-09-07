//
// Created by Mike Smith on 05/09/2017.
//

#ifndef DRAUGHTS_BOARD_H
#define DRAUGHTS_BOARD_H

#include <functional>
#include "Piece.h"

class Board
{
public:
    static constexpr int numberOfRows = 10;
    static constexpr int numberOfColumns = 10;
    
    using Square = Piece;
    using Grid = Square[numberOfRows][numberOfColumns];
    
    struct Position
    {
        int row;
        int col;
        
        bool operator==(const Position &rhs) const { return row == rhs.row && col == rhs.col; }
        Position upperLeft() const { return {row - 1, col - 1}; }
        Position bottomLeft() const { return {row + 1, col - 1}; }
        Position upperRight() const { return {row - 1, col + 1}; }
        Position bottomRight() const { return {row + 1, col + 1}; }
    };

private:
    Grid _board;

public:
    static bool isInBoard(const Position &pos);
    static bool isAtEdge(const Position &pos, const Piece &piece);
    
    void setupBoard();
    const Piece &getPiece(const Position &pos) const;
    void placePiece(const Position &pos, const Piece &piece);
    void removePiece(const Position &pos);
    void promotePiece(const Position &pos);
    bool isEmpty(const Position &pos) const;
};

#endif  // DRAUGHTS_BOARD_H
