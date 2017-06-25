#include "mainwindow.h"
#include <QApplication>

#include <iostream>

int main(int argc, char *argv[])
{
	setlocale (LC_ALL,"");
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
