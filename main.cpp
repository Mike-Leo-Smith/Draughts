#include <QApplication>
#include <QDebug>
#include "Network/Connection.h"
#include "GameController.h"
#include "LoginDialog.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    LoginDialog login;
    login.exec();
    if (login.result() == QDialog::Rejected) {
        exit(-1);
    }

    auto *connection = login.connection();

    GameController w(connection);
    w.show();

    return a.exec();
}
