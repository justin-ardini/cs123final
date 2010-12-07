#include <QtGui/QApplication>
#include <iostream>
#define GL_GLEXT_LEGACY // no glext.h, we have our own
#include <qgl.h>
#include <pty.h>
#include "mainwindow.h"
using std::cout;
using std::endl;

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
