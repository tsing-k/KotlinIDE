#-------------------------------------------------
#
# Project created by QtCreator 2017-06-14T21:54:15
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = KotlinIDE
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
        codeeditor.cpp \
    highlighter.cpp

HEADERS  += mainwindow.h \
        codeeditor.h \
    highlighter.h

FORMS    += mainwindow.ui

RESOURCES += \
    resource.qrc
