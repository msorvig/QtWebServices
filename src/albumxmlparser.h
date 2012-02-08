
#include "picasatypes.h"

#include <QtXml>

#ifndef ALBUMXMLPARSER_H
#define ALBUMXMLPARSER_H

class AlbumXmlHandler : public QXmlDefaultHandler
{
public:
    AlbumXmlHandler();
    QtPicasaAlbum parseAlbumXml(const QByteArray &xml);
private:
    bool fatalError (const QXmlParseException & exception);
    bool startElement ( const QString & namespaceURI, const QString & localName, const QString & qName, const QXmlAttributes & atts );
    bool characters ( const QString & string );
    bool endElement ( const QString & namespaceURI, const QString & localName, const QString & qName );

    QtPicasaAlbum album;
    QtPicasaImage currentImage;
    enum Mode { Base, AlbumTitle, Group, ImageTitle, ImageDescription };
    QStack<Mode> mode;
};

#endif
