//
// Created by Mike Smith on 05/09/2017.
//

#ifndef DRAUGHTS_DRAUGHTS_H
#define DRAUGHTS_DRAUGHTS_H

#include <cstdint>
#include <vector>
#include "Board.h"
#include "Path.h"

class Draughts
{
public:
    enum class GameState {
        whiteWins,
        blackWins,
        inGame,
        draw,
    };

    using PathList = std::vector<Path>;
    using Position = Board::Position;
    using MoveList = std::vector<Position>;
    using PieceList = std::vector<Position>;

private:
    Board _board;
    GameState _gameState;
    Player _currentPlayer;
    PathList _availablePaths;
    MoveList _availableMoves;
    PieceList _availablePieces;
    PathNode *_currentNode;

    void _getAvailablePaths();
    void _clearAvailablePaths();
    void _getAvailablePieces();
    void _clearAvailablePieces();
    void _getAvailableMoves();
    void _clearAvailableMoves();
    void _makeMove(const Draughts::Position &curr, const Draughts::Position &next, const Position &capture);
    void _tryPromotePiece();

public:
    Draughts();
    void reset();

    const PieceList &availablePieces() const;
    bool selectPiece(const Position &piecePos);
    const MoveList & availableMoves() const;
    bool selectMove(const Position &nextPos);
    bool isTurnEnded() const;
    bool startNewTurn();
    const Board &board() const;
    void endGame(GameState state);
    GameState state() const;
    Player currentPlayer() const;

    void loadGame(const Board &board, Player currentPlayer);
};

#endif  // DRAUGHTS_DRAUGHTS_H
