INCLUDEPATH += $$PWD
SOURCES += $$PWD/qtgoogleclientlogin.cpp \
           $$PWD/qtpicasaweb.cpp \
           $$PWD/qtpicasaloader.cpp \
           $$PWD/feedxmlparser.cpp \
           $$PWD/albumxmlparser.cpp \
           $$PWD/picasatypes.cpp \
           $$PWD/blockingnetworkaccessmanager.cpp \
           $$PWD/qts3.cpp \

HEADERS += $$PWD/qtgoogleclientlogin.h  \
           $$PWD/qtpicasaweb.h \
           $$PWD/qtpicasaloader.h \
           $$PWD/feedxmlparser.h \
           $$PWD/albumxmlparser.h \
           $$PWD/picasatypes.h \
           $$PWD/blockingnetworkaccessmanager.h \
           $$PWD/qts3.h \

include ($$PWD/../3rdparty/libaws/libaws.pri)

QT += xml
