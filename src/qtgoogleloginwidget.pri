INCLUDEPATH += $$PWD

osx {
    OBJECTIVE_SOURCES += $$PWD/qtwebview_osx.mm
    HEADERS += $$PWD/qtwebview.h
    LIBS += -framework WebKit -framework AppKit

    OBJECTIVE_SOURCES += $$PWD/qtgoogleloginwidget.mm
    HEADERS += $$PWD/qtgoogleloginwidget.h
    QT += widgets
}
