#include "quamodbus.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QUaModbus w;
    w.show();
    return a.exec();
}
