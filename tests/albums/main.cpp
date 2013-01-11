#include <QtWidgets/QApplication>
#include <QtCore>

#include "applogic.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    AppLogic logic;
    logic.start();

    return app.exec();
}


