#include "picasabrowser.h"
#include "ui_picasabrowser.h"
#include <QtCore>

PicasaBrowser::PicasaBrowser(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::PicasaBrowser)
{
    ui->setupUi(this);

    qtPicasaWeb = new QtPicasaWeb;

    // testing
    QFile albumXml(":/xml/albumlisting.txt");
    albumXml.open(QIODevice::ReadOnly);
    qtPicasaWeb->parseAlbumXml(albumXml.readAll());
}

PicasaBrowser::~PicasaBrowser()
{
    delete ui;
}
