#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.showFullScreen();

    a.setFont(QFont("Calibri",15));
    return a.exec();
}
/* СМОТРИ mainwindow.cpp */
