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
    _sounds["Background"]->play();

    if (_connection->isServer()) {
        auto randomPlayer = PlayerHelper::random();
        _setPlayer(randomPlayer);
        _sendPlayerMessage(PlayerHelper::opponent(randomPlayer));
        _waitForNewGame();
    }

    connect(_connection, &Connection::messageArrived, this, &GameController::_handleMessage);
    connect(ui->drawButton, &QPushButton::clicked, this, &GameController::_askForDraw);
    connect(ui->surrenderButton, &QPushButton::clicked, this, &GameController::_surrender);
    connect(ui->draughtsView, &DraughtsView::clickedOnBoard, this, &GameController::_handleClick);
    connect(ui->sendButton, &QPushButton::clicked, this, &GameController::_handleChat);

#if ALLOW_CHEATING_MODE
    if (isCheatingModeEnabled()) {
        disconnect(ui->sendButton, &QPushButton::clicked, this, &GameController::_handleChat);
        ui->sendButton->setText("Cheat");
        ui->sendButton->setCheckable(true);
        ui->sendingText->setDisabled(true);
        connect(ui->sendButton, &QPushButton::clicked, this, [this] {
            _isEditingBoard = ui->sendButton->isChecked();
            if (_isEditingBoard) {
                _editableBoard = _game.board();
                _sendCheatMessage("begin");
                _updateCheatView();
            } else {
                _game.loadGame(_editableBoard, _player);
                _currentPosition = { -1, -1 };
                _sendCheatMessage("end", { -1, -1 }, _player);
                _updateView();
            }
        });
    }
#endif
}

GameController::~GameController()
{
    delete ui;
}

#if ALLOW_CHEATING_MODE
void GameController::keyPressEvent(QKeyEvent *event)
{
    if (isCheatingModeEnabled() && _isEditingBoard && Board::isPositionValid(_currentPosition)) {
        auto piece = _editableBoard.getPiece(_currentPosition);
        auto key = event->key();
        switch (key) {
        case Qt::Key_E: piece.color = PieceColor::empty; break;
        case Qt::Key_B: piece.color = PieceColor::black; break;
        case Qt::Key_W: piece.color = PieceColor::white; break;
        case Qt::Key_C: piece.type = PieceType::crowned; break;
        case Qt::Key_N: piece.type = PieceType::normal; break;
        default: break;
        }
        _editableBoard.placePiece(_currentPosition, piece);
        _sendCheatMessage("editing", _currentPosition, piece.color, piece.type);
        _updateCheatView();
    }
}
#endif

void GameController::_handleChat()
{
    if (!ui->sendingText->text().isEmpty()) {
        _sendChatMessage(ui->sendingText->text());
        _displayNewChat("[You] " + ui->sendingText->text());
        ui->sendingText->clear();
    }
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
    addSound("Background", "qrc:/Sounds/Background.mp3");
}

void GameController::_updateView()
{
    ui->draughtsView->setCurrentPiecePosition(_currentPosition);
    ui->draughtsView->setLocalPlayer(_player);
    ui->draughtsView->setCurrentPlayer(_game.currentPlayer());
    ui->draughtsView->setBoard(_game.board());
    ui->draughtsView->setAvailableMoves(_game.availableMoves());
    ui->draughtsView->setAvailablePieces(_game.availablePieces());
    ui->draughtsView->update();
}

#if ALLOW_CHEATING_MODE
void GameController::_updateCheatView()
{
    ui->draughtsView->setCurrentPiecePosition(_currentPosition);
    ui->draughtsView->setLocalPlayer(_player);
    ui->draughtsView->setCurrentPlayer(Player::empty);
    ui->draughtsView->setBoard(_editableBoard);
    ui->draughtsView->setAvailableMoves({});
    ui->draughtsView->setAvailablePieces({});
    ui->draughtsView->update();
}
#endif

void GameController::_displayNewChat(QString s)
{
    if (_lastChat.isEmpty()) {
        ui->receivedText->setText(s);
    } else {
        ui->receivedText->setText(_lastChat + "   " + s);
    }
    _lastChat = s;
}

void GameController::_handleMessage(QString message) {

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
#if ALLOW_CHEATING_MODE
    else if (stringList[0] == "[Cheat]" && isCheatingModeEnabled()) {
        _handleCheatMessage(stringList);
    }
#endif
}

void GameController::_waitForNewGame()
{
    _sendReadyMessage("inform");
    setDisabled(true);
}

void GameController::_startNewGame()
{
    qDebug() << "New Game";
    _sounds["Starting"]->play();
    _player = PlayerHelper::opponent(_player);
    _game.reset();
    _currentPosition = { -1, -1 };
    setDisabled(false);
    _updateView();
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
        _sendSurrenderMessage();
        _handleGameOver("You lose!", "You surrendered.");
        _waitForNewGame();
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

#if ALLOW_CHEATING_MODE
void GameController::_sendCheatMessage(QString status, const Position &position, PieceColor color, PieceType type)
{
    QString posStr = QString("%1 %2").arg(position.row).arg(position.col);
    QString colorStr = QString::fromStdString(PieceHelper::toString(color));
    QString typeStr = (type == PieceType::crowned) ? "crowned" : "normal";
    _connection->sendMessage("[Cheat] " + status + " " + posStr + " " + colorStr + " " + typeStr);
    qDebug() << posStr << colorStr << typeStr;
}
#endif

void GameController::_handlePlayerMessage(const QStringList &list)
{
    if (list.size() != 2) {
        return;
    }

    auto player = PlayerHelper::fromString(list[1].toStdString());
    _setPlayer(player);
}

void GameController::_handleMoveMessage(const QStringList &list)
{
    if (list.size() != 3) {
        return;
    }

    Position position = { list[1].toInt(), list[2].toInt() };
    _game.selectMove(position);
    _handlePieceMove(position);
}

void GameController::_handleSelectMessage(const QStringList &list)
{
    if (list.size() != 3) {
        return ;
    }

    Position position = { list[1].toInt(), list[2].toInt() };
    _game.selectPiece(position);
    _handlePieceSelection(position);
}

void GameController::_handleReadyMessage(const QStringList &list)
{
    qDebug() << list;

    if (list.size() != 2) {
        return;
    }

    _startNewGame();

    if (list[1] == "inform") {
        _sendReadyMessage("answer");
    }
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
    qDebug() << list;

    if (list.size() != 1) {
        return;
    }

    if (_player == Player::black) {
        _game.endGame(GameState::blackWins);
    } else {
        _game.endGame(GameState::whiteWins);
    }
    _handleGameOver("You win!", "Your opponent surrendered.");
}

void GameController::_handleDrawMessage(const QStringList &list)
{
    if (list.size() != 2) {
        return;
    }

    if (list[1] == "ask") {
        auto answer = QMessageBox::information(this, "Draw", "Your opponent asked for a draw.\nDo you agree?", QMessageBox::Ok, QMessageBox::Cancel);
        if (answer == QMessageBox::Ok) {
            _sendDrawMessage("accept");
            _game.endGame(GameState::draw);
            _handleGameOver("Draw!");
            _waitForNewGame();
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
        _handleGameOver("Draw!", "Your opponent accepted your request for a draw.");
    }
}

#if ALLOW_CHEATING_MODE
void GameController::_handleCheatMessage(const QStringList &list)
{
    if (!isCheatingModeEnabled() || list.size() != 6) {
        return;
    }

    qDebug() << list;

    Position pos = { list[2].toInt(), list[3].toInt() };
    auto color = PieceHelper::fromString(list[4].toStdString());
    auto type = (list[5] == "crowned") ? PieceType::crowned : PieceType::normal;

    if (list[1] == "begin") {
        _isEditingBoard = true;
        _editableBoard = _game.board();
        ui->sendButton->setChecked(true);
        _updateCheatView();
        return;
    }

    if (list[1] == "end") {
        _isEditingBoard = false;
        _game.loadGame(_editableBoard, color);
        _currentPosition = { -1, -1 };
        ui->sendButton->setChecked(false);
        _updateCheatView();
        return;
    }

    _editableBoard.placePiece(pos, { color, type });
    _currentPosition = pos;
    _updateCheatView();
}
#endif

void GameController::_handleClick(const Position &position)
{
#if ALLOW_CHEATING_MODE
    if (isCheatingModeEnabled() && _isEditingBoard && Board::isPositionValid(position)) {
        _currentPosition = position;
        _updateCheatView();
        return;
    }
#endif

    if (_game.currentPlayer() != _player) {
        return;
    }

    if (_game.selectPiece(position)) {
        _sendSelectMessage(position);
        _handlePieceSelection(position);
    } else if (_game.selectMove(position)) {
        _sendMoveMessage(position);
        _handlePieceMove(position);
    }
}

void GameController::_handleGameOver(QString state, QString additionalDescription)
{
    QString gameOverInfo = additionalDescription + "\n" + state;

    if (state == "Draw!") {
        _sounds["Draw"]->play();
    } else if (state == "You win!") {
        _sounds["Winning"]->play();
    } else {
        _sounds["Losing"]->play();
    }
    QMessageBox::information(this, "Result", gameOverInfo, QMessageBox::Ok);
}

void GameController::_handlePieceMove(const Position &pos)
{
    _sounds["Moving"]->play();

    _currentPosition = pos;

    if (_game.isTurnEnded()) {
        _game.startNewTurn();
    }

    _updateView();

    if (_game.state() != GameState::inGame) {
        if ((_game.state() == GameState::whiteWins && _player == Player::white) ||
            (_game.state() == GameState::blackWins && _player == Player::black)) {
            _handleGameOver("You win!");
            _waitForNewGame();
        } else {
            _handleGameOver("You lose!");
            setDisabled(true);
        }
    }
}

void GameController::_handlePieceSelection(const Position &pos)
{
    _sounds["Selecting"]->play();
    _currentPosition = pos;
    _updateView();
}
