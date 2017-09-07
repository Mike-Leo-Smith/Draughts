#include <QDebug>
#include <QTimer>
#include <QStringList>
#include <random>
#include "Game.h"
#include "ui_Draughts.h"

Game::Game(Connection *connection, QWidget *parent) :
    QWidget(parent), ui(new Ui::GameWindow), _connection(connection)
{
    ui->setupUi(this);

    if (_connection != nullptr) {
        connect(_connection, &Connection::messageArrived, this, [this](QString message) {
            _decodeMessage(message);
        });

        connect(ui->sendButton, &QPushButton::clicked, _connection, [this] {
            if (!ui->sendingText->text().isEmpty()) {
                auto sendingMessage = ui->sendingText->text();
                _connection->sendMessage(_encodeMessage("Chat", sendingMessage));
                _displayNewMessage(sendingMessage);
            }
        });

        if (_connection->isServer()) {
            qDebug() << _connection->isServer();
            int randomPlayer = std::rand() % 2;
            if (randomPlayer == 0) {
                ui->draughtsView->setPlayer(Player::white);
            } else {
                ui->draughtsView->setPlayer(Player::black);
            }
            _connection->sendMessage(_encodeMessage("Player", QString::number(randomPlayer)));
        }

        connect(ui->draughtsView, &DraughtsView::pieceMoved, _connection, [this](Board::Position to) {
            _connection->sendMessage(_encodeMessage("Move", QString("%1 %2").arg(to.row).arg(to.col)));
        });

        connect(ui->draughtsView, &DraughtsView::pieceSelected, _connection, [this](Board::Position pos) {
            _connection->sendMessage(_encodeMessage("Select", QString("%1 %2").arg(pos.row).arg(pos.col)));
        });
    }
}

void Game::_displayNewMessage(QString s)
{
    auto lastMessage = _getLastMessage();
    _setLastMessage(s);

    if (lastMessage.isEmpty()) {
        ui->receivedText->setText("- " + s);
    } else {
        ui->receivedText->setText("- " + lastMessage + "  - " + s);
    }
}

void Game::_setLastMessage(QString s)
{
    _locker.lockForWrite();
    _lastMessage = s;
    _locker.unlock();
}

QString Game::_getLastMessage()
{
    _locker.lockForRead();
    QString lastMessage = _lastMessage;
    _locker.unlock();
    return lastMessage;
}

QString Game::_encodeMessage(QString type, QString data)
{
    return "[" + type + "] " + data;
}

void Game::_decodeMessage(QString message) {
    qDebug() << message;
    auto stringList = message.split(" ", QString::SkipEmptyParts);

    if (stringList.isEmpty()) {
        return;
    }

    if (stringList[0] == "[Chat]") {
        _displayNewMessage(message.remove("[Chat] "));
        return;
    }

    if (stringList[0] == "[Player]") {
        qDebug() <<  message;
        auto player = (stringList[1].toInt() == 0) ? Player::black : Player::white;
        ui->draughtsView->setPlayer(player);
        return;
    }

    if (stringList[0] == "[Move]") {
        Board::Position to = { stringList[1].toInt(), stringList[2].toInt() };
        ui->draughtsView->moveEnemyPiece(to);
        return;
    }

    if (stringList[0] == "[Select]") {
        Board::Position pos = { stringList[1].toInt(), stringList[2].toInt() };
        ui->draughtsView->selectEnemyPiece(pos);
    }
}

Game::~Game()
{
    delete ui;
}
