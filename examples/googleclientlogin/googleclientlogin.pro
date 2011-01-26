include (../../src/src.pri)

QT       += core gui network

TARGET = QtGoogleAPI
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    qtclientlogin.cpp

HEADERS  += mainwindow.h \
    qtclientlogin.h

FORMS    += mainwindow.ui
