#include <QDebug>
#include <QTimer>
#include <QStringList>
#include <random>
#include <QMessageBox>
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
                ui->sendingText->clear();
                _displayNewMessage(sendingMessage);
            }
        });

        if (_connection->isServer()) {
            int randomPlayer = std::rand() % 2;
            if (randomPlayer == 0) {
                ui->draughtsView->setPlayer(Player::white);
            } else {
                ui->draughtsView->setPlayer(Player::black);
            }
            _connection->sendMessage(_encodeMessage("Player", QString::number(randomPlayer)));
            _connection->sendMessage(_encodeMessage("Ready", "inform"));
            setDisabled(true);
        }

        connect(ui->draughtsView, &DraughtsView::pieceMoved, _connection, [this](Board::Position to) {
            _connection->sendMessage(_encodeMessage("Move", QString("%1 %2").arg(to.row).arg(to.col)));
        });

        connect(ui->draughtsView, &DraughtsView::pieceSelected, _connection, [this](Board::Position pos) {
            _connection->sendMessage(_encodeMessage("Select", QString("%1 %2").arg(pos.row).arg(pos.col)));
        });

        connect(ui->drawButton, &QPushButton::clicked, _connection, [this] {
            auto answer = QMessageBox::warning(this, "Draw", "Are you sure to ask for a draw?", QMessageBox::Ok, QMessageBox::Cancel);
            if (answer == QMessageBox::Ok) {
                _connection->sendMessage(_encodeMessage("Draw", "ask"));
                setDisabled(true);
            }
        });

        connect(ui->surrenderButton, &QPushButton::clicked, _connection, [this] {
           auto answer = QMessageBox::warning(this, "Surrender", "Are you sure to surrender?", QMessageBox::Ok, QMessageBox::Cancel);
           if (answer == QMessageBox::Ok) {
               _connection->sendMessage(_encodeMessage("Surrender"));
               ui->draughtsView->endGame("You lose!", "You surrendered.\n");
               setDisabled(true);
           }
        });

        connect(ui->draughtsView, &DraughtsView::gameEnded, _connection, [this](QString state) {
           ui->draughtsView->endGame(state);
            _connection->sendMessage(_encodeMessage("Ready", "inform"));
            setDisabled(true);
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
        _connection->sendMessage(_encodeMessage("Ready", "inform"));
        setDisabled(true);
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
        return;
    }

    if (stringList[0] == "[Surrender]") {
        ui->draughtsView->endGame("You win!", "Your opponent surrendered.\n");
        _connection->sendMessage(_encodeMessage("Ready", "inform"));
        setDisabled(true);
        return;
    }

    if (stringList[0] == "[Draw]") {
        if (stringList[1] == "ask") {
            auto answer = QMessageBox::information(this, "Draw", "Your opponent asked for a draw.\nDo you agree?", QMessageBox::Ok, QMessageBox::Cancel);
            if (answer == QMessageBox::Ok) {
                _connection->sendMessage(_encodeMessage("Draw", "accept"));
                ui->draughtsView->endGame("Draw!");
                _connection->sendMessage(_encodeMessage("Ready", "inform"));
                setDisabled(true);
            } else {
                _connection->sendMessage(_encodeMessage("Draw", "reject"));
            }
            return;
        }

        if (stringList[1] == "reject") {
            QMessageBox::information(this, "Draw", "Your request for a draw is rejected by your opponent.", QMessageBox::Ok);
            setDisabled(false);
            return;
        }

        if (stringList[1] == "accept") {
            ui->draughtsView->endGame("Draw!", "Your oppenent accepted your request for a draw.\n");
            _connection->sendMessage(_encodeMessage("Ready", "inform"));
            setDisabled(true);
        }
        return;
    }

    if (stringList[0] == "[Ready]") {

        ui->draughtsView->startNewGame();
        setDisabled(false);

        if (stringList[1] == "inform") {
            _connection->sendMessage(_encodeMessage("Ready", "answer"));
        }
    }
}

Game::~Game()
{
    delete ui;
}
