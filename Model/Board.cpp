//
// Created by Mike Smith on 05/09/2017.
//

#include "Board.h"

bool Board::isInBoard(const Board::Position &pos)
{
    return pos.row >= 0 && pos.row < numberOfRows && pos.col >= 0 && pos.col < numberOfColumns;
}

void Board::setupBoard()
{
    // Clear up the board.
    for (auto &row : _board) {
        for (auto &square : row) {
            square.color = PieceColor::empty;
            square.type = PieceType::normal;
        }
    }

    // Place black pieces.
    for (int row = 0; row < 4; row++) {
        auto startingCol = (row % 2 == 0) ? 1 : 0;
        for (int col = startingCol; col < numberOfColumns; col += 2) {
            _board[row][col].color = PieceColor::black;
        }
    }

    // Place white pieces.
    for (int row = numberOfRows - 4; row < numberOfRows; row++) {
        auto startingCol = (row % 2 == 0) ? 1 : 0;
        for (int col = startingCol; col < numberOfColumns; col += 2) {
            _board[row][col].color = PieceColor::white;
        }
    }
}

// Assumed that the row and col are valid, and that there are no conflicts.
const Piece &Board::getPiece(const Board::Position &pos) const
{
    return _board[pos.row][pos.col];
}

// Assumed that the row and col are valid, and that there are no conflicts.
void Board::placePiece(const Board::Position &pos, const Piece &piece)
{
    _board[pos.row][pos.col] = piece;
}

// Assumed that the row and col are valid and the piece exists.
void Board::removePiece(const Board::Position &pos)
{
    _board[pos.row][pos.col].color = PieceColor::empty;
}

bool Board::isEmpty(const Board::Position &pos) const
{
    return isInBoard(pos) && getPiece(pos).color == PieceColor::empty;
}

// Assumed that the row and col are valid, and that the piece exists.
void Board::promotePiece(const Board::Position &pos)
{
    _board[pos.row][pos.col].type = PieceType::crowned;
}

bool Board::isAtEdge(const Board::Position &pos, const Piece &piece)
{
    return (piece.color == PieceColor::white) ? (pos.row == 0) : (pos.row == Board::numberOfRows - 1);
}
