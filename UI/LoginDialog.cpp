#include <QMessageBox>
#include <QtConcurrent/QtConcurrent>
#include <QPropertyAnimation>
#include "LoginDialog.h"
#include "ui_Login.h"

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent), ui(new Ui::LoginDialog), _connection(new Connection)
{
    ui->setupUi(this);
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

#if ALLOW_CHEATING_MODE
        if (port == cheating_port) {
            if (ip.isEmpty()) {
                ip = "127.0.0.1";
            }
            setCheatingModeEnabled(true);
        } else {
            setCheatingModeEnabled(false);
        }
#endif

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

LoginDialog::~LoginDialog()
{
    delete ui;
    delete _animation;
}

Connection *LoginDialog::connection()
{
    return _connection;
}
