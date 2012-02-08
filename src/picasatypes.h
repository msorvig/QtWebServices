#include <QtCore>

#ifndef PICASATYPES_H
#define PICASATYPES_H

class QtPicasaImage
{
public:
    QString title;
    QString description;
    QString url;
    QString thumbnailUrl;
};

// corresponds to one picasa album
class QtPicasaAlbum
{
public:
    QString title;
    QList<QtPicasaImage> images;
};

#endif
