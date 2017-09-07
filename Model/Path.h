//
// Created by Mike Smith on 05/09/2017.
//

#ifndef DRAUGHTS_PATH_H
#define DRAUGHTS_PATH_H

#include <vector>
#include "Board.h"

using NodeList = std::vector<class PathNode *>;

class PathNode
{
public:
    using Position = Board::Position;

private:
    Position _position;
    Position _capturePosition;
    PathNode *_parent;
    NodeList _children;
    int _numberOfCaptures;
    
    void _updateNumberOfCaptures(int newCaptures);

public:
    PathNode(const Position &pos, PathNode *parent, int numberOfCaptures, const Position &captured);
    PathNode(PathNode &&) = default;
    ~PathNode();
    int numberOfCaptures() const;
    void addChild(PathNode *node);
    void prune();
    const NodeList &children() const;
    const Position &position() const;
    const Position &captured() const;
    bool isEnd() const;
};

class PathFinder
{
public:
    using Position = Board::Position;
    using PathTree = PathNode *;

private:
    static bool _findCapturePaths(Board &board, PathNode *node, const Position &position, const Piece &piece);
    static bool _findNonCapturePaths(Board &board, PathNode *root, const Position &position, const Piece &piece);

public:
    PathFinder() = delete;
    static PathTree findPathsForPiece(Board board, const Position &pos);
};

using Path = PathFinder::PathTree;

#endif  // DRAUGHTS_PATH_H
