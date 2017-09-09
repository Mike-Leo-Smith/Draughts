#ifndef DRAUGHTSVIEW_H
#define DRAUGHTSVIEW_H

#include <QWidget>
#include <QMediaPlayer>
#include "../Model/Draughts.h"

class DraughtsView : public QWidget
{
    Q_OBJECT

    using Position = Board::Position;
    using GameState = Draughts::GameState;

private:
    QPointF _offset;
    qreal _boardLength;
    qreal _squareLength;
    qreal _outerGridLineWidth = 5;
    qreal _innerGridLineWidth = 1;
    qreal _pieceEdgeLineWidth = 1;
    qreal _pieceRadius;
    QColor _innerGridColor = QColor(70, 30, 10);
    QColor _outerGridColor = _innerGridColor.darker();
    QColor _lightBackgroundColor = QColor(240, 200, 170);
    QColor _darkBackgroundColor = QColor(178, 107, 90);
    QColor _pieceEdgeColor = QColor(218, 152, 130);
    QColor _lightPieceColor = QColor(220, 250, 255);
    QColor _darkPieceColor = QColor(50, 30, 10);

    Board _board;
    Player _localPlayer = Player::empty;
    Player _currentPlayer = Player::empty;
    Position _currentPiecePosition = { -1, -1 };
    Draughts::MoveList _availableMoves;
    Draughts::PieceList _availablePieces;

    void _drawGrids();
    void _drawPieces();

    Position _convertPointToPosition(const QPointF point) const;
    QPointF _convertPositionToPoint(const Position &position) const;

public:
    explicit DraughtsView(QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

    void setBoard(const Board &board);
    void setLocalPlayer(Player player);
    void setCurrentPlayer(Player player);
    void setCurrentPiecePosition(const Position &position);
    void setAvailableMoves(const Draughts::MoveList moves);
    void setAvailablePieces(const Draughts::PieceList pieces);

signals:
    void clickedOnBoard(Position position);
};

#endif // DRAUGHTSVIEW_H
