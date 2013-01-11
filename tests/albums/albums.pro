include (../../../qtmacextras/src/qtmacextras.pri)
include (../../src/src.pri)
include (../../src/qtgoogleloginwidget.pri)


QT       += core gui network  xml

TARGET = albums
TEMPLATE = app
CONFIG -= app_bundle

HEADERS += applogic.h
SOURCES += main.cpp applogic.cpp

