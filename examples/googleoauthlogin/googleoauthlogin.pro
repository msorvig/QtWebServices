include (../../src/src.pri)
include (../../../qtmacextras/src/qtmacextras.pri)

OBJECTS_DIR=.obj
MOC_DIR=.moc

QT += core gui network widgets

TARGET = googleoauthlogin
TEMPLATE = app

HEADERS += qtwebview.h loginwidget.h applogic.h
OBJECTIVE_SOURCES += qtwebview.mm
SOURCES += main.cpp loginwidget.cpp applogic.cpp

LIBS += -framework WebKit
