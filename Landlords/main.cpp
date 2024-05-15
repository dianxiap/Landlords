#include "gamepanel.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    GamePanel w;
    w.show();
    return a.exec();
}
