#-------------------------------------------------
#
# Project created by QtCreator 2010-11-05T14:00:55
#
#-------------------------------------------------

QT += core gui opengl

TARGET = cs123final
TEMPLATE = app
CONFIG += console

SOURCES += main.cpp \
    mainwindow.cpp \
    glwidget.cpp \
    drawengine.cpp \
    targa.cpp \
    glm.cpp \
    terrain.cpp \
    camera.cpp \
    CS123Vector.inl \
    CS123Matrix.inl \
    CS123Matrix.cpp

HEADERS += mainwindow.h \
    glwidget.h \
    drawengine.h \
    targa.h \
    glm.h \
    common.h \
    terrain.h \
    glext.h \
    camera.h \
    CS123Vector.h \
    CS123Matrix.h \
    CS123Algebra.h \
    CS123Common.h

FORMS += mainwindow.ui
INCLUDEPATH += src
DEPENDPATH += src

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3
RESOURCES +=
