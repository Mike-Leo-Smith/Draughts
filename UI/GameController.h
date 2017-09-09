#ifndef GAME_H
#define GAME_H

#include <QWidget>
#include <QString>
#include <QStringList>
#include <QMediaPlayer>
#include <QKeyEvent>
#include <QMap>

#include "../Model/Draughts.h"
#include "../Network/Connection.h"
#include "../CheatingConfig.h"

namespace Ui {
    class GameWindow;
}

class GameController : public QWidget
{
    Q_OBJECT

    using Position = Board::Position;
    using GameState = Draughts::GameState;

public:
    explicit GameController(Connection *cnnection, QWidget *parent = 0);
    ~GameController() override;

#if ALLOW_CHEATING_MODE
    void keyPressEvent(QKeyEvent *event) override;
#endif

private:

    Ui::GameWindow *ui;

    // Member for network connection.
    Connection *_connection = nullptr;

    // Member for chatting.
    QString _lastChat;

    // Sound effect table.
    QMap<QString, QMediaPlayer *> _sounds;

    // Members related to game playing.
    Draughts _game;
    Player _player = Player::empty;
    Position _currentPosition = { -1, -1 };

#if ALLOW_CHEATING_MODE
    // Cheating...
    Board _editableBoard;
    bool _isEditingBoard = false;
    void _sendCheatMessage(QString status, const Position &position = { -1, -1 }, PieceColor color = PieceColor::empty, PieceType type = PieceType::normal);
    void _handleCheatMessage(const QStringList &list);
    void _updateCheatView();
#endif

    // Method for loading sounds.
    void _loadSounds();

    // Method for updating view.
    void _updateView();

    // Methods for chatting.
    void _displayNewChat(QString s);


private slots:
    // Methods related to game playing.
    void _waitForNewGame();
    void _startNewGame();
    void _setPlayer(Player player);
    void _surrender();
    void _askForDraw();
    void _handlePieceMove(const Position &pos);
    void _handlePieceSelection(const Position &pos);
    void _handleGameOver(QString state, QString additionalDescription = "");

    // Methods related to message sending and handling.
    void _handleMessage(QString message);

    void _sendPlayerMessage(Player player);
    void _sendMoveMessage(const Position &to);
    void _sendSelectMessage(const Position &at);
    void _sendReadyMessage(QString detail);
    void _sendChatMessage(QString content);
    void _sendSurrenderMessage();
    void _sendDrawMessage(QString detail);

    void _handlePlayerMessage(const QStringList &list);
    void _handleMoveMessage(const QStringList &list);
    void _handleSelectMessage(const QStringList &list);
    void _handleReadyMessage(const QStringList &list);
    void _handleChatMessage(const QStringList &list);
    void _handleSurrenderMessage(const QStringList &list);
    void _handleDrawMessage(const QStringList &list);
    void _handleClick(const Position &position);
    void _handleChat();
};

#endif // GAME_H
