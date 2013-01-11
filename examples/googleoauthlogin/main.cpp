#include <QtWidgets/QtWidgets>
#include <qtmaccocoaviewcontainer.h>

#include "applogic.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    AppLogic appLogic;
    appLogic.start();

    return a.exec();
}
