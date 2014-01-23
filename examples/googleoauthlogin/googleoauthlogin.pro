include (../../src/src.pri)
include (../../src/qtgoogleloginwidget.pri)
include (../../src/qtgoogleloginwindow.pri)


OBJECTS_DIR=.obj
MOC_DIR=.moc

QT += core gui network widgets

TARGET = googleoauthlogin
TEMPLATE = app

HEADERS +=  applogic.h
SOURCES += main.cpp  applogic.cpp

