#-------------------------------------------------
#
# Project created by QtCreator 2010-11-05T14:00:55
#
#-------------------------------------------------

QT       += core gui opengl

TARGET = lab09
TEMPLATE = app
CONFIG += console

SOURCES += main.cpp\
        mainwindow.cpp \
    glwidget.cpp \
    drawengine.cpp \
    targa.cpp \
    glm.cpp \
    terrain.cpp

HEADERS  += mainwindow.h \
    glwidget.h \
    drawengine.h \
    targa.h \
    glm.h \
    common.h \
    terrain.h

FORMS    += mainwindow.ui

RESOURCES +=
