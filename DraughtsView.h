#ifndef DRAUGHTSVIEW_H
#define DRAUGHTSVIEW_H

#include <QWidget>
#include <Model/Draughts.h>

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

    Draughts _game;
    Player _player = Player::empty;
    Position _currentPiecePosition = { -1, -1 };

    void _drawGrids();
    void _drawPieces();

    Position _convertPointToPosition(const QPointF point) const;
    QPointF _convertPositionToPoint(const Position &position) const;

public:
    explicit DraughtsView(QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void setPlayer(Player player);
    void testGameOver();

signals:
    void pieceMoved(Position to);
    void pieceSelected(Position pos);

public slots:
    void moveEnemyPiece(Position to);
    void selectEnemyPiece(Position pos);
};

#endif // DRAUGHTSVIEW_H
