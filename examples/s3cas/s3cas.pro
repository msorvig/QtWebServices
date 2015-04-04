TEMPLATE = app
TARGET = s3cas

CONFIG -= app_bundle
QMAKE_CXXFLAGS += -std=c++11
QT = core network

# Comment in to to enable parallel file transfers
include (../../../qtconcurrent2/qtconcurrent2.pri)
include (../../src/qts3.pri)
include (../../src/qts3cas.pri)

OBJECTS_DIR = .obj
MOC_DIR = .moc
RCC_DIR = .rcc

SOURCES += main.cpp
