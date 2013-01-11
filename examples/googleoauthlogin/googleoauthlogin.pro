include (../../src/src.pri)
include (../../src/qtgoogleloginwidget.pri)
include (../../../qtmacextras/src/qtmacextras.pri)

OBJECTS_DIR=.obj
MOC_DIR=.moc

QT += core gui network widgets

TARGET = googleoauthlogin
TEMPLATE = app

HEADERS +=  applogic.h
SOURCES += main.cpp  applogic.cpp

LIBS += -framework WebKit
