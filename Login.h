#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>
#include <QPropertyAnimation>
#include "Network/Connection.h"

namespace Ui {
    class LoginDialog;
}

class Login : public QDialog
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = 0);
    ~Login();

    Connection *connection();

private:
    Ui::LoginDialog *ui;
    Connection *_connection = nullptr;
    QPropertyAnimation *_animation = nullptr;
};

#endif // LOGIN_H
