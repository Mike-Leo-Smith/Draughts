//
// Created by Mike Smith on 05/09/2017.
//

#include <iostream>
#include "Draughts.h"

Draughts::Draughts()
{
    reset();
}

void Draughts::reset()
{
    _currentPlayer = Player::black;
    _gameState = GameState::inGame;
    _board.setupBoard();
    _currentNode = nullptr;
    startNewTurn();
}

void Draughts::_getAvailablePaths()
{
    int maxCaptureCount = 0;
    _clearAvailablePaths();

    for (int row = 0; row < Board::numberOfRows; row++) {
        for (int col = 0; col < Board::numberOfColumns; col++) {
            Position position = { row, col };
            if (_board.getPiece(position).color != _currentPlayer) {
                continue;
            }

            if (auto path = PathFinder::findPathsForPiece(_board, position)) {
                if (path->numberOfCaptures() > maxCaptureCount) {
                    _clearAvailablePaths();
                    _availablePaths.push_back(path);
                    maxCaptureCount = path->numberOfCaptures();
                } else if (path->numberOfCaptures() == maxCaptureCount) {
                    _availablePaths.push_back(path);
                }
            }
        }
    }
}

const Draughts::PieceList &Draughts::availablePieces() const
{
    return _availablePieces;
}

bool Draughts::selectPiece(const Position &piecePos)
{
    for (auto path : _availablePaths) {
        if (path->position() == piecePos) {
            _currentNode = path;
            _getAvailableMoves();
            return true;
        }
    }
    return false;
}

const Draughts::MoveList &Draughts::availableMoves() const
{
    return _availableMoves;
}

bool Draughts::selectMove(const Position &nextPos)
{
    if (_currentNode == nullptr) {
        return false;
    }

    for (auto node : _currentNode->children()) {
        if (node->position() == nextPos) {
            _makeMove(_currentNode->position(), node->position(), node->captured());
            _clearAvailablePieces();
            _currentNode = node;
            _getAvailableMoves();

            if (node->isEnd()) {
                _tryPromotePiece();
            }
            return true;
        }
    }
    return false;
}

void Draughts::_clearAvailablePieces()
{
    _availablePieces.clear();
}

void Draughts::_getAvailablePieces()
{
    _clearAvailablePieces();
    for (auto path : _availablePaths) {
        _availablePieces.push_back(path->position());
    }
}

void Draughts::_clearAvailablePaths()
{
    for (auto path : _availablePaths) {
        delete path;
    }
    _availablePaths.clear();
}

void Draughts::_makeMove(const Position &curr, const Position &next, const Position &capture)
{
    if (_board.isInBoard(capture)) {
        _board.removePiece(capture);
    }

    _board.placePiece(next, _board.getPiece(curr));
    _board.removePiece(curr);
}

void Draughts::_getAvailableMoves()
{
    _clearAvailableMoves();

    if (_currentNode == nullptr) {
        return;
    }

    for (auto node : _currentNode->children()) {
        _availableMoves.push_back(node->position());
    }
}

void Draughts::_clearAvailableMoves()
{
    _availableMoves.clear();
}

bool Draughts::isTurnEnded() const
{
    return _currentNode == nullptr || _currentNode->isEnd();
}

void Draughts::_tryPromotePiece()
{
    const auto &currentPos = _currentNode->position();
    if (Board::isAtEdge(currentPos, _board.getPiece(currentPos))) {
        _board.promotePiece(currentPos);
    }
}

bool Draughts::startNewTurn()
{
    if (isTurnEnded()) {
        _currentNode = nullptr;
        _currentPlayer = (_currentPlayer == Player::white) ? Player::black : Player::white;
        _getAvailablePaths();
        _getAvailablePieces();
        if (_availablePaths.empty()) {
            _gameState = (_currentPlayer == Player::white) ? GameState::blackWins : GameState::whiteWins;
        }
        return true;
    }
    return false;
}

Player Draughts::player() const
{
    return _currentPlayer;
}

const Board &Draughts::board() const
{
    return _board;
}

Draughts::GameState Draughts::state() const
{
    return _gameState;
}
