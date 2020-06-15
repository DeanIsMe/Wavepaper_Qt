#include "mainwindow.h"
#include "datatypes.h"
#include <QApplication>

State_S state;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
