
#include "picasatypes.h"

#include <QtXml>
#include <QtCore/qxmlstream.h>

#ifndef ALBUMXMLPARSER_H
#define ALBUMXMLPARSER_H

class AlbumXmlHandler: public QXmlStreamReader
{
public:
    AlbumXmlHandler();
    QtPicasaAlbum parseAlbumXml(const QByteArray &xml);
private:
    void parseFeed();
    void parseEntry();
    void parseLink();
    void parseMediaGroup(QtPicasaImage *image);
    void parseThumbnail(QtPicasaImage *image);

    QtPicasaAlbum album;
};

#endif
