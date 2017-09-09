#include <QDebug>
#include <QTimer>
#include <QStringList>
#include <random>
#include <QMessageBox>
#include "GameController.h"
#include "ui_Draughts.h"

GameController::GameController(Connection *connection, QWidget *parent) :
    QWidget(parent), ui(new Ui::GameWindow), _connection(connection)
{
    ui->setupUi(this);

    _loadSounds();

    if (_connection->isServer()) {
        auto randomPlayer = PlayerHelper::random();
        _setPlayer(randomPlayer);
        _sendPlayerMessage(PlayerHelper::opponent(randomPlayer));
        _waitForNewGame();
    }

    connect(_connection, &Connection::messageArrived, this, &GameController::_decodeMessage);
    connect(ui->drawButton, &QPushButton::clicked, this, &GameController::_askForDraw);
    connect(ui->surrenderButton, &QPushButton::clicked, this, &GameController::_surrender);
    connect(ui->draughtsView, &DraughtsView::clickedOnBoard, this, &GameController::_handleClick);
    connect(ui->sendButton, &QPushButton::clicked, _connection, [this] {
        if (!ui->sendingText->text().isEmpty()) {
            _sendChatMessage(ui->sendingText->text());
            _displayNewChat("[You] " + ui->sendingText->text());
            ui->sendingText->clear();
        }
    });
}

GameController::~GameController()
{
    delete ui;
}

void GameController::_loadSounds()
{
    auto addSound = [this](QString id, QString path) {
        auto sound = new QMediaPlayer(this);
        sound->setMedia(QUrl(path));
        _sounds.insert(id, sound);
    };

    addSound("Moving", "qrc:/Sounds/Moving.mp3");
    addSound("Selecting", "qrc:/Sounds/Selecting.mp3");
    addSound("Winning", "qrc:/Sounds/Winning.wav");
    addSound("Losing", "qrc:/Sounds/Failing.wav");
    addSound("Starting", "qrc:/Sounds/Starting.wav");
    addSound("Draw", "qrc:/Sounds/Draw.wav");
}

void GameController::_updateView(const Position &currPos)
{
    ui->draughtsView->setCurrentPiecePosition(currPos);
    ui->draughtsView->setLocalPlayer(_player);
    ui->draughtsView->setCurrentPlayer(_game.currentPlayer());
    ui->draughtsView->setBoard(_game.board());
    ui->draughtsView->setAvailableMoves(_game.availableMoves());
    ui->draughtsView->setAvailablePieces(_game.availablePieces());
    ui->draughtsView->update();
}

void GameController::_displayNewChat(QString s)
{
    if (_lastChat.isEmpty()) {
        ui->receivedText->setText(s);
    } else {
        ui->receivedText->setText(_lastChat + "   " + s);
    }
    _lastChat = s;
}

void GameController::_decodeMessage(QString message) {

    auto stringList = message.split(" ", QString::SkipEmptyParts);

    if (stringList.isEmpty()) {
        return;
    }

    if (stringList[0] == "[Chat]") {
        _handleChatMessage(stringList);
    } else if (stringList[0] == "[Player]") {
        _handlePlayerMessage(stringList);
    } else if (stringList[0] == "[Move]") {
        _handleMoveMessage(stringList);
    } else if (stringList[0] == "[Select]") {
        _handleSelectMessage(stringList);
    } else if (stringList[0] == "[Surrender]") {
        _handleSurrenderMessage(stringList);
    } else if (stringList[0] == "[Draw]") {
        _handleDrawMessage(stringList);
    } else if (stringList[0] == "[Ready]") {
        _handleReadyMessage(stringList);
    }
}

void GameController::_waitForNewGame()
{
    _sendReadyMessage("inform");
    setDisabled(true);
}

void GameController::_startNewGame()
{
    _sounds["Starting"]->play();
    _player = PlayerHelper::opponent(_player);
    _game.reset();
    setDisabled(false);
    _updateView({-1, -1});
}

void GameController::_surrender()
{
    auto answer = QMessageBox::warning(this, "Surrender", "Are you sure to surrender?", QMessageBox::Ok, QMessageBox::Cancel);
    if (answer == QMessageBox::Ok) {
        if (_player == Player::white) {
            _game.endGame(GameState::blackWins);
        } else {
            _game.endGame(GameState::whiteWins);
        }
        _handleGameOver("You surrendered.");
    }
}

void GameController::_askForDraw()
{
    auto answer = QMessageBox::warning(this, "Draw", "Are you sure to ask for a draw?", QMessageBox::Ok, QMessageBox::Cancel);
    if (answer == QMessageBox::Ok) {
        _sendDrawMessage("ask");
        setDisabled(true);
    }
}

void GameController::_setPlayer(Player player)
{
    _player = player;
}

void GameController::_sendPlayerMessage(Player player)
{
    _connection->sendMessage("[Player] " + QString::fromStdString(PlayerHelper::toString(player)));
}

void GameController::_sendMoveMessage(const Position &to)
{
    _connection->sendMessage(QString("[Move] %1 %2").arg(to.row).arg(to.col));
}

void GameController::_sendSelectMessage(const Position &at)
{
    _connection->sendMessage(QString("[Select] %1 %2").arg(at.row).arg(at.col));
}

void GameController::_sendReadyMessage(QString detail)
{
    _connection->sendMessage("[Ready] " + detail);
}

void GameController::_sendChatMessage(QString content)
{
    _connection->sendMessage("[Chat] " + content);
}

void GameController::_sendSurrenderMessage()
{
    _connection->sendMessage("[Surrender]");
}

void GameController::_sendDrawMessage(QString detail)
{
    _connection->sendMessage("[Draw] " + detail);
}

void GameController::_handlePlayerMessage(const QStringList &list)
{
    if (list.size() != 2) {
        qDebug() << list;
        return;
    }

    auto player = PlayerHelper::fromString(list[1].toStdString());
    _setPlayer(player);
}

void GameController::_handleMoveMessage(const QStringList &list)
{
    if (list.size() != 3) {
        qDebug() << list;
        return;
    }

    Position position = { list[1].toInt(), list[2].toInt() };
    _game.selectMove(position);
    _handlePieceMove(position);
}

void GameController::_handleSelectMessage(const QStringList &list)
{
    if (list.size() != 3) {
        qDebug() << list;
        return ;
    }

    Position position = { list[1].toInt(), list[2].toInt() };
    _game.selectPiece(position);
    _handlePieceSelection(position);
}

void GameController::_handleReadyMessage(const QStringList &list)
{
    if (list.size() != 2) {
        qDebug() << list;
        return;
    }

    if (list[1] == "inform") {
        _sendReadyMessage("answer");
    }

    _startNewGame();
}

void GameController::_handleChatMessage(const QStringList &list)
{
    if (list.size() < 2) {
        qDebug() << list;
        return;
    }

    QString content = "[Opponent] ";
    for (auto iter = list.cbegin() + 1; iter != list.cend(); iter++) {
        content.append(*iter);
    }
    _displayNewChat(content);
}

void GameController::_handleSurrenderMessage(const QStringList &list)
{
    if (list.size() != 1) {
        qDebug() << list;
        return;
    }

    if (_player == Player::black) {
        _game.endGame(GameState::blackWins);
    } else {
        _game.endGame(GameState::whiteWins);
    }
    _handleGameOver("Your opponent surrendered.");
}

void GameController::_handleDrawMessage(const QStringList &list)
{
    if (list.size() != 2) {
        qDebug() << list;
        return;
    }

    if (list[1] == "ask") {
        auto answer = QMessageBox::information(this, "Draw", "Your opponent asked for a draw.\nDo you agree?", QMessageBox::Ok, QMessageBox::Cancel);
        if (answer == QMessageBox::Ok) {
            _sendDrawMessage("accept");
            _game.endGame(GameState::draw);
            _handleGameOver();
        } else {
            _sendDrawMessage("reject");
        }
        return;
    }

    if (list[1] == "reject") {
        QMessageBox::information(this, "Draw", "Your request for a draw is rejected by your opponent.", QMessageBox::Ok);
        setDisabled(false);
        return;
    }

    if (list[1] == "accept") {
        _game.endGame(GameState::draw);
        _handleGameOver();
    }
}

void GameController::_handleClick(const Position &position)
{
    if (_game.currentPlayer() != _player) {
        return;
    }

    if (_game.selectPiece(position)) {
        _handlePieceSelection(position);
        _sendSelectMessage(position);
    } else if (_game.selectMove(position)) {
        _handlePieceMove(position);
        _sendMoveMessage(position);
    }
}

void GameController::_handleGameOver(QString additionalDescription)
{
    QString gameOverInfo = additionalDescription + "\n";

    if (_game.state() == GameState::draw) {
        gameOverInfo += "Draw!";
        _sounds["Draw"]->play();
    } else if ((_game.state() == GameState::whiteWins && _player == Player::white) ||
               (_game.state() == GameState::blackWins && _player == Player::black)) {
        gameOverInfo += "You win!";
        _sounds["Winning"]->play();
    } else {
        gameOverInfo += "You lose!";
        _sounds["Losing"]->play();
    }
    QMessageBox::information(this, "Result", gameOverInfo, QMessageBox::Ok);

    _waitForNewGame();
}

void GameController::_handlePieceMove(const Position &pos)
{
    _sounds["Moving"]->play();

    if (_game.isTurnEnded()) {
        _game.startNewTurn();
    }

    if (_game.state() != GameState::inGame) {
        _handleGameOver();
    }

    _updateView(pos);
}

void GameController::_handlePieceSelection(const Position &pos)
{
    _sounds["Selecting"]->play();
    _updateView(pos);
}
