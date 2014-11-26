TEMPLATE = app
TARGET = s3cas

CONFIG -= app_bundle
QMAKE_CXXFLAGS += -std=c++11
QT = core

# Comment in to to enable parallel file transfers
include (../../../qtconcurrent2/qtconcurrent2.pri)
include (../../src/s3.pri)

OBJECTS_DIR = .obj
MOC_DIR = .moc
RCC_DIR = .rcc

SOURCES += main.cpp
