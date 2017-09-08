#ifndef GAME_H
#define GAME_H

#include <QWidget>
#include <Network/Connection.h>
#include <QReadWriteLock>

namespace Ui {
    class GameWindow;
}

class Game : public QWidget
{
    Q_OBJECT

public:
    explicit Game(Connection *cnnection, QWidget *parent = 0);
    ~Game() override;

private:
    Ui::GameWindow *ui;
    Connection *_connection = nullptr;
    QString _lastMessage;
    QReadWriteLock _locker;

    void _setLastMessage(QString s);
    QString _getLastMessage();
    void _displayNewMessage(QString s);
    void _decodeMessage(QString message);
    QString _encodeMessage(QString type, QString data = "");
};

#endif // GAME_H
