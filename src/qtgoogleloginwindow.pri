INCLUDEPATH += $$PWD

HEADERS += $$PWD/qtwebview.h
osx {
    OBJECTIVE_SOURCES += $$PWD/qtwebview_osx.mm
    LIBS += -framework WebKit -framework AppKit
}
ios {
    OBJECTIVE_SOURCES += $$PWD/qtwebview_ios.mm
    LIBS += -framework UIKit
}

SOURCES += $$PWD/qtgoogleloginwindowcontroller.cpp
HEADERS += $$PWD/qtgoogleloginwindowcontroller.h



