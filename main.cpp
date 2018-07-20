#include <QApplication>
#include "mainwindow.h"

// 1.0.1

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
//    w.resize(1000,500);
    
    return a.exec();
}
