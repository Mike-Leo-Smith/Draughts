#ifndef CONNECTION_H
#define CONNECTION_H

#include <QObject>
#include <QString>
#include <QTcpSocket>
#include <QTcpServer>

class Connection : public QObject
{
    Q_OBJECT

private:
    QTcpServer *_server = nullptr;
    QTcpSocket *_socket = nullptr;
    bool _isServer = false;

public:
    static constexpr int timeLimitation = 10000;

    explicit Connection(QObject *parent = nullptr);
    static QString localAddress();
    bool isServer() const;

signals:
    void connectionCreated();
    void connectionFailed();
    void messageArrived(QString message);

public slots:
    void sendMessage(QString message);
    void receiveMessage();

    void connectToClient(int port);
    void connectToServer(QString address, int port);
};

#endif // CONNECTION_H
