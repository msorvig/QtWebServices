TEMPLATE = app

include (../../src/s3.pri)
QT = core
QMAKE_CXXFLAGS += -std=c++11

# Comment in to to enable parallel file transfers
# include (../../../qtconcurrent2/qtconcurrent2.pri)

CONFIG -= app_bundle
TARGET = s3

OBJECTS_DIR = .obj
MOC_DIR = .moc

SOURCES += main.cpp

