include ($$PWD/../../src/qts3.pri)

QT = core network testlib

TARGET = tst_qts3
TEMPLATE = app
CONFIG -= app_bundle
QMAKE_MACOSX_DEPLOYMENT_TARGET=10.8

SOURCES += $$PWD/../../src/qts3_tst.cpp
