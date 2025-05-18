#include "mainwindow.h"
#include <windows.h>
#include <QApplication>
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    SetDllDirectory(L".");
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
