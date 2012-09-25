TEMPLATE = app

include (../../src/s3.pri)
QT = core

CONFIG -= app_bundle
TARGET = s3


OBJECTS_DIR = .obj
MOC_DIR = .moc

SOURCES += main.cpp

