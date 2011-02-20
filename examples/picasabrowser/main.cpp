#include <QtGui/QApplication>
#include "picasabrowser.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    PicasaBrowser w;
    w.show();

    return a.exec();
}
