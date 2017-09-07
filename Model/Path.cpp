//
// Created by Mike Smith on 05/09/2017.
//

#include <algorithm>
#include "Path.h"

PathFinder::PathTree PathFinder::findPathsForPiece(Board board, const Position &pos)
{
    auto piece = board.getPiece(pos);
    auto pathTreeRoot = new PathNode(pos, nullptr, 0, { -1, -1 });

    // Pick the piece that is to move.
    board.removePiece(pos);

    if (_findCapturePaths(board, pathTreeRoot, pos, piece) || _findNonCapturePaths(board, pathTreeRoot, pos, piece)) {
        pathTreeRoot->prune();
        return pathTreeRoot;
    }

    // No valid moves, delete the search tree.
    delete pathTreeRoot;
    return nullptr;
}

bool PathFinder::_findCapturePaths(Board &board, PathNode *node, const Position &position, const Piece &piece)
{
    auto moves = { &Position::upperLeft, &Position::upperRight, &Position::bottomLeft, &Position::bottomRight };
    auto captureColor = (piece.color == PieceColor::white) ? PieceColor::black : PieceColor::white;

    auto canCapture = false;
    auto currCaptureCount = node->numberOfCaptures();

    for (auto move : moves) {
        auto capturePos = position;
        int limit = (piece.type == PieceType::normal) ? 1 : Board::numberOfRows;

        // Find the first position that there is a piece lying in the required direction.
        for (int counter = 0; counter < limit; counter++) {
            if (!board.isEmpty(capturePos)) {
                break;
            }
            capturePos = (capturePos.*move)();
        }

        if (board.isInBoard(capturePos) && board.getPiece(capturePos).color == captureColor) {
            auto targetPos = (capturePos.*move)();
            auto capturedPiece = board.getPiece(capturePos);

            // Remove the captured piece.
            board.removePiece(capturePos);

            // A piece which is able to be captured requires the square behind it to be empty.
            for (int counter = 0; counter < limit; counter++) {
                if (!board.isEmpty(targetPos)) {
                    break;
                }
                canCapture = true;
                auto newNode = new PathNode(targetPos, node, currCaptureCount + 1, capturePos);
                node->addChild(newNode);
                _findCapturePaths(board, newNode, targetPos, piece);
                targetPos = (targetPos.*move)();
            }

            // Place the captured piece back for the next turn of search.
            board.placePiece(capturePos, capturedPiece);
        }
    }

    return canCapture;
}

bool PathFinder::_findNonCapturePaths(Board &board, PathNode *root, const Position &position, const Piece &piece)
{
    auto movesForWhite = { &Position::upperLeft, &Position::upperRight };
    auto movesForBlack = { &Position::bottomLeft, &Position::bottomRight };
    auto movesForCrowned = { &Position::upperLeft, &Position::upperRight,
                             &Position::bottomLeft, &Position::bottomRight };

    decltype(movesForCrowned) moves;

    if (piece.type == PieceType::crowned) {
        moves = movesForCrowned;
    } else if (piece.color == PieceColor::white) {
        moves = movesForWhite;
    } else {
        moves = movesForBlack;
    }

    auto canMove = false;

    for (auto move : moves) {
        auto target = (position.*move)();
        int limit = (piece.type == PieceType::normal) ? 1 : Board::numberOfRows;
        for (int counter = 0; counter < limit; counter++) {
            if (!board.isEmpty(target)) {
                break;
            }
            root->addChild(new PathNode(target, root, 0, { -1, -1 }));
            canMove = true;
            target = (target.*move)();
        }
    }

    return canMove;
}

PathNode::PathNode(const Position &pos, PathNode *parent, int numberOfCaptures, const Position &captured)
    : _position(pos), _capturePosition(captured), _parent(parent), _numberOfCaptures(numberOfCaptures) {}

PathNode::~PathNode()
{
    for (auto node : _children) {
        delete node;
    }
}

void PathNode::addChild(PathNode *node)
{
    _children.push_back(node);
    _updateNumberOfCaptures(node->_numberOfCaptures);
}

void PathNode::prune()
{
    auto numberOfCaptures = _numberOfCaptures;

    // Remove all sub-nodes that have fewer captures.
    _children.erase(std::remove_if(_children.begin(), _children.end(), [&numberOfCaptures](PathNode *node) {
                        if (node->_numberOfCaptures < numberOfCaptures) {
                            delete node;
                            return true;
                        }
                        return false;
                    }), _children.end());

    // Prune children recursively.
    for (auto node : _children) {
        node->prune();
    }
}

void PathNode::_updateNumberOfCaptures(int newCaptures)
{
    if (newCaptures > _numberOfCaptures) {
        _numberOfCaptures = newCaptures;
        if (_parent != nullptr) {
            _parent->_updateNumberOfCaptures(newCaptures);
        }
    }
}

int PathNode::numberOfCaptures() const
{
    return _numberOfCaptures;
}

const NodeList &PathNode::children() const
{
    return _children;
}

const PathNode::Position &PathNode::position() const
{
    return _position;
}

bool PathNode::isEnd() const
{
    return _children.empty();
}

const PathNode::Position &PathNode::captured() const
{
    return _capturePosition;
}
