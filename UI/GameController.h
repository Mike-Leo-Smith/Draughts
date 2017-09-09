#ifndef GAME_H
#define GAME_H

#include <QWidget>
#include <Network/Connection.h>
#include <QReadWriteLock>
#include <QString>
#include <QStringList>
#include <QMediaPlayer>
#include <QMap>

#include "../Model/Draughts.h"
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

private:

    Ui::GameWindow *ui;
    Connection *_connection = nullptr;

    // Members for chatting.
    QString _lastChat;

    // Sound effect table.
    QMap<QString, QMediaPlayer *> _sounds;

    // Members related to game playing.
    Draughts _game;
    Player _player;

    // Method for loading sounds.
    void _loadSounds();

    // Methods related to game playing.
    void _waitForNewGame();
    void _startNewGame();
    void _setPlayer(Player player);
    void _surrender();
    void _askForDraw();
    void _handlePieceMove(const Position &pos);
    void _handlePieceSelection(const Position &pos);
    void _handleGameOver(QString additionalDescription = "");

    // Method for updating view.
    void _updateView(const Position &currPos);

    // Methods for chatting.
    void _displayNewChat(QString s);

    // Methods related to message sending and handling.
    void _decodeMessage(QString message);

    void _sendPlayerMessage(Player player);
    void _sendMoveMessage(const Position &to);
    void _sendSelectMessage(const Position &at);
    void _sendReadyMessage(QString detail);
    void _sendChatMessage(QString content);
    void _sendSurrenderMessage();
    void _sendDrawMessage(QString detail);

    void _handleMessage(const QString &raw);
    void _handlePlayerMessage(const QStringList &list);
    void _handleMoveMessage(const QStringList &list);
    void _handleSelectMessage(const QStringList &list);
    void _handleReadyMessage(const QStringList &list);
    void _handleChatMessage(const QStringList &list);
    void _handleSurrenderMessage(const QStringList &list);
    void _handleDrawMessage(const QStringList &list);

private slots:
    void _handleClick(const Position &position);
};

#endif // GAME_H
