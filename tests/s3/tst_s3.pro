include ($$PWD/../../src/qts3.pri)

QT += core network testlib

TARGET = tst_qts3
TEMPLATE = app
CONFIG -= app_bundle

SOURCES += $$PWD/../../src/qts3_tst.cpp
