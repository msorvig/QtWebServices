
#include "picasatypes.h"

#include <QtXml>

#ifndef FEEDXMLPARSER_H
#define FEEDXMLPARSER_H

class FeedXmlHandler : public QXmlDefaultHandler
{
public:
    FeedXmlHandler();
    QtPicasaFeed parseFeedXml(const QByteArray &xml);
private:
    bool fatalError (const QXmlParseException & exception);
    bool startElement ( const QString & namespaceURI, const QString & localName, const QString & qName, const QXmlAttributes & atts );
    bool characters ( const QString & string );
    bool endElement ( const QString & namespaceURI, const QString & localName, const QString & qName );

    QtPicasaFeed feed;
    QtPicasaAlbumMeta currentAlbum;

    QStack<QString> localNames;
    QStack<QString> qualifiedNames;
};

#endif
