#include <QHostInfo>
#include <QDebug>
#include <QTimer>
#include <QMessageBox>
#include "Connection.h"

Connection::Connection(QObject *parent)
    : QObject(parent) {}

void Connection::connectToClient(int port) {

    _isServer = true;

    delete _socket;
    delete _server;

    _socket = nullptr;
    _server = new QTcpServer;
    _server->listen(QHostAddress::Any, port);

    if (_server->waitForNewConnection(timeLimitation)) {
        _server->moveToThread(thread());
        _socket = _server->nextPendingConnection();
        connect(_socket, &QTcpSocket::readyRead, this, &Connection::receiveMessage);
        emit connectionCreated();
        return;
    }

    delete _server;
    _server = nullptr;
    emit connectionFailed();
}

void Connection::connectToServer(QString address, int port)
{
    _isServer = false;

    delete _server;
    delete _socket;

    _socket = new QTcpSocket;
    _server = nullptr;

    _socket->connectToHost(QHostAddress(address), port);
    if (_socket->waitForConnected(timeLimitation)) {
        _socket->moveToThread(thread());
        connect(_socket, &QTcpSocket::readyRead, this, &Connection::receiveMessage);
        emit connectionCreated();
        return;
    }

    delete _socket;
    _socket = nullptr;
    emit connectionFailed();
}

void Connection::receiveMessage()
{
    if (_socket != nullptr) {
        while (!_socket->atEnd()) {
            emit messageArrived(_socket->readLine().trimmed());
        }
    }
}

void Connection::sendMessage(QString message)
{
    if (_socket != nullptr) {
        _socket->write(message.append("\n").toUtf8());
    }
}

QString Connection::localAddress()
{
    auto info = QHostInfo::fromName(QHostInfo::localHostName());
    QString localIp;
    for (auto address : info.addresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol) {
            localIp = address.toString();
            qDebug() << localIp;
        }
    }
    return localIp;
}

bool Connection::isServer() const {
    return _isServer;
}
