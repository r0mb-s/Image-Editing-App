#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    MainWindow w;
    // w.setWindowFlags(Qt::FramelessWindowHint);
    w.show();
    w.move(0, 0);
    w.resize(800, 600);
    return a.exec();
}
