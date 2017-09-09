#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <QMessageBox>
#include "DraughtsView.h"

DraughtsView::DraughtsView(QWidget *parent)
    : QWidget(parent) {}

void DraughtsView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    _boardLength = std::min(width(), height()) - _outerGridLineWidth;
    _squareLength = _boardLength / Board::numberOfRows;
    _offset = QPointF(_outerGridLineWidth / 2, _outerGridLineWidth / 2);
    _pieceRadius = 0.43 * _squareLength;

    _drawGrids();
    _drawPieces();
}

void DraughtsView::_drawGrids()
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    painter.translate(_offset);

    QBrush darkSquareBrush(_darkBackgroundColor);
    QBrush lightSquareBrush(_lightBackgroundColor);

    // Draw squares.
    painter.setPen(QPen(_innerGridColor, _innerGridLineWidth));
    for (int row = 0; row < Board::numberOfRows; row++) {
        for (int col = 0; col < Board::numberOfColumns; col++) {
            if (row % 2 == col % 2) {
                painter.setBrush(lightSquareBrush);
            } else {
                painter.setBrush(darkSquareBrush);
            }
            painter.drawRect(col * _squareLength, row * _squareLength, _squareLength, _squareLength);
        }
    }

    // Draw outer grids.
    painter.setPen(QPen(_outerGridColor, _outerGridLineWidth));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(0, 0, _boardLength, _boardLength);
}

void DraughtsView::_drawPieces()
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    painter.translate(_offset);

    QBrush lightPieceBrush(_lightPieceColor);
    QBrush darkPieceBrush(_darkPieceColor);
    QPen pen(_pieceEdgeColor, _pieceEdgeLineWidth);

    auto getPieceCenter = [this](const Position &position) {
        return _convertPositionToPoint(position) + QPointF(0.5 * _squareLength, 0.5 * _squareLength);
    };

    for (int row = 0; row < Board::numberOfRows; row++) {
        for (int col = 0; col < Board::numberOfColumns; col++) {

            painter.setPen(pen);

            Position position = { row, col };
            auto center = getPieceCenter(position);

            const auto &piece = _board.getPiece(position);
            if (piece.color == PieceColor::empty) {
                continue;
            }

            if (piece.color == PieceColor::white) {
                painter.setBrush(lightPieceBrush);
            } else {
                painter.setBrush(darkPieceBrush);
            }
            painter.drawEllipse(center, _pieceRadius, _pieceRadius);

            if (piece.type == PieceType::crowned) {
                painter.setPen(Qt::NoPen);
                painter.setBrush(QColor(102, 192, 138, 197));
                painter.drawEllipse(center, _pieceRadius * 0.3, _pieceRadius * 0.3);
            }
        }
    }

    if (_localPlayer == _currentPlayer) {
        for (const auto &piecePosition : _availablePieces) {
            painter.setPen(QPen(QColor(135, 130, 190), 3));
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(getPieceCenter(piecePosition), _pieceRadius, _pieceRadius);
        }
        for (const auto &move : _availableMoves) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(Qt::red);
            painter.drawEllipse(getPieceCenter(move), _pieceRadius * 0.1, _pieceRadius * 0.1);
        }
    }

    if (Board::isInBoard(_currentPiecePosition)) {
        painter.setPen(QPen(_pieceEdgeColor.darker(), 5));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(getPieceCenter(_currentPiecePosition), _pieceRadius, _pieceRadius);
    }
}

QPointF DraughtsView::_convertPositionToPoint(const Position &position) const
{
    if (_localPlayer == Player::black) {
        return QPointF((Board::numberOfColumns - 1 - position.col) * _squareLength, (Board::numberOfRows - 1 - position.row) * _squareLength);
    } else {
        return QPointF(position.col * _squareLength, position.row  * _squareLength);
    }
}

DraughtsView::Position DraughtsView::_convertPointToPosition(const QPointF point) const
{
    auto row = static_cast<int>((point.y() - _offset.y()) / _squareLength);
    auto col = static_cast<int>((point.x() - _offset.x()) / _squareLength);

    if (_localPlayer == Player::white) {
        return { row, col };
    } else {
        return { Board::numberOfRows - 1 - row, Board::numberOfColumns - 1 - col };
    }
}

void DraughtsView::mousePressEvent(QMouseEvent *event)
{
    Position position = _convertPointToPosition(event->localPos());
    emit clickedOnBoard(position);
}

void DraughtsView::setLocalPlayer(Player player)
{
    _localPlayer = player;
}

void DraughtsView::setBoard(const Board &board)
{
    _board = board;
}

void DraughtsView::setCurrentPiecePosition(const Position &position)
{
    _currentPiecePosition = position;
}

void DraughtsView::setAvailableMoves(const Draughts::MoveList moves)
{
    _availableMoves = moves;
}

void DraughtsView::setAvailablePieces(const Draughts::PieceList pieces)
{
    _availablePieces = pieces;
}

void DraughtsView::setCurrentPlayer(Player player)
{
    _currentPlayer = player;
}
