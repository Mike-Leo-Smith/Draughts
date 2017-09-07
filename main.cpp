#include <QApplication>
#include <QDebug>
#include "Network/Connection.h"
#include "Game.h"
#include "Login.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Login login;
    login.exec();
    if (login.result() == QDialog::Rejected) {
        exit(-1);
    }

    auto *connection = login.connection();

    Game w(connection);
    w.show();

    return a.exec();
}
