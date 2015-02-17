INCLUDEPATH += $$PWD

HEADERS +=  \
    $$PWD/qts3.h \
    $$PWD/qts3_p.h \
    $$PWD/qts3cas.h \

SOURCES += \
    $$PWD/qts3.cpp \
    $$PWD/qts3cas.cpp \

# libaws
include ($$PWD/../3rdparty/libaws/libaws.pri)

mac {
    # dependencies for libaws through macports
    INCLUDEPATH += /opt/local/include/libxml2
    LIBS+= -L/opt/local/lib -lxml2 -lcrypto -lcurl
} else {
    INCLUDEPATH += /usr/include/libxml2
    LIBS+= -L/usr/lib -lxml2 -lcrypto -lcurl
}

