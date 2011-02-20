include (../../src/src.pri)
QT       += core gui network xmlpatterns xml

TARGET = picasabrowser
TEMPLATE = app


SOURCES += main.cpp\
        picasabrowser.cpp \
        ../../src/qtpicasaweb.cpp

HEADERS  += picasabrowser.h \
        ../../src/qtpicasaweb.h

FORMS    += picasabrowser.ui

OTHER_FILES += \
    albumlisting.txt \
    photolisting.txt

RESOURCES += \
    testxml.qrc
