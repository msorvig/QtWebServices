#include <QtGui/QGuiApplication>
#include "qtquick2applicationviewer.h"

#include <qtwindowcontrolleritem.h>
#include <qtnativewebviewitem.h>

#include <QtQml/QQmlExtensionPlugin>
#include <QtQml/qqml.h>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // OS X hackery
    // setenv("QT_MAC_OPENGL_SURFACE_ORDER", "-1", 0);
    // setenv("QT_MAC_WANTS_LAYER", "1", 0);

    QtQuick2ApplicationViewer viewer;

    qmlRegisterType<QtWindowControllerItem>("QtNativeWebView", 1, 0, "WindowController");
    qmlRegisterType<QtNativeWebViewItem>("QtNativeWebView", 1, 0, "NativeWebView");

    viewer.setMainQmlFile(QStringLiteral("qml/qmloathlogin/main.qml"));
    viewer.showExpanded();

    return app.exec();
}
