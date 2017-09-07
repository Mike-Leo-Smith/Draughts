#include <QMessageBox>
#include <QtConcurrent/QtConcurrent>
#include <QPropertyAnimation>
#include "Login.h"
#include "ui_Login.h"

Login::Login(QWidget *parent) :
    QDialog(parent), ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    _connection = new Connection;
    _animation = new QPropertyAnimation(this);

    connect(ui->serverButton, &QRadioButton::clicked, this, [this] {
        ui->ipInput->setText(Connection::localAddress());
        ui->ipInput->setReadOnly(true);
    });

    connect(ui->clientButton, &QRadioButton::clicked, this, [this] {
        ui->ipInput->clear();
        ui->ipInput->setReadOnly(false);
    });

    connect(_connection, &Connection::connectionCreated, this, [this] {
        setDisabled(false);
        _animation->stop();
        ui->connectionProgress->setValue(100);
        accept();
    });

    connect(_connection, &Connection::connectionFailed, this, [this] {
        setDisabled(false);
        _animation->stop();
        ui->connectionProgress->setValue(0);
        QMessageBox::warning(this, "Warning", "Connection failed!", QMessageBox::Ok);
    });

    connect(ui->connectButton, &QPushButton::clicked, this, [this] {
        auto ip = ui->ipInput->text();
        auto port = ui->portInput->text().toInt();
        auto isClient = ui->clientButton->isChecked();
        setDisabled(true);

        _animation->setTargetObject(ui->connectionProgress);
        _animation->setPropertyName("value");
        _animation->setStartValue(0);
        _animation->setEndValue(100);
        _animation->setDuration(Connection::timeLimitation);
        _animation->start();

        QtConcurrent::run([this, ip, port, isClient] {
            if (isClient) {
                _connection->connectToServer(ip, port);
            } else {
                _connection->connectToClient(port);
            }
        });
    });
}

Login::~Login()
{
    delete ui;
}

Connection *Login::connection()
{
    return _connection;
}
