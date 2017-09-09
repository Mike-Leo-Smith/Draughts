#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>
#include <QPropertyAnimation>
#include "Network/Connection.h"
#include "CheatingConfig.h"

namespace Ui {
    class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = 0);
    ~LoginDialog() override;

    Connection *connection();

private:
    Ui::LoginDialog *ui;
    Connection *_connection = nullptr;
    QPropertyAnimation *_animation = nullptr;

};

#endif // LOGIN_H
