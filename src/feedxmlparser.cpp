#include "feedxmlparser.h"

QtPicasaFeed FeedXmlHandler::parseFeedXml(const QByteArray &xml)
{
    qDebug() << "QtPicasaWeb::parseFeedXml got" << xml.count() << "bytes";

    feed = QtPicasaFeed();

    QBuffer xmlBuffer;
    xmlBuffer.setData(xml);
    QXmlSimpleReader xmlReader;
    QXmlInputSource *source = new QXmlInputSource(&xmlBuffer);
    xmlReader.setContentHandler(this);
    xmlReader.setErrorHandler(this);

    bool ok = xmlReader.parse(source);
    if (!ok)
        qDebug() << "xml parse ok" << ok;

    return feed;
}

FeedXmlHandler::FeedXmlHandler()
    :QXmlDefaultHandler()
{

}

bool FeedXmlHandler::fatalError (const QXmlParseException & exception)
{
    qWarning() << "Fatal error on line" << exception.lineNumber()
               << ", column" << exception.columnNumber() << ":"
               << exception.message() << exception.publicId();

    return true;
}

bool FeedXmlHandler::startElement ( const QString & namespaceURI, const QString & localName, const QString & qualifiedName, const QXmlAttributes & atts )
{
    //qDebug() << "element" << localName << namespaceURI << qualifiedName;
    localNames.push(localName);
    qualifiedNames.push(qualifiedName);

    if (qualifiedName == QLatin1String("entry")) {
        //qDebug() << "push";
        currentAlbum = QtPicasaAlbumMeta();
    }


    return true;
}

bool FeedXmlHandler::characters ( const QString & string )
{
    if (localNames.top() == QLatin1String("title") && qualifiedNames.top() == QLatin1String("media:title")) {
        //qDebug() << "title" << string << localNames.size() << localNames << qualifiedNames.top();
        currentAlbum.title = string;
    }
    if (localNames.top() == QLatin1String("id") && qualifiedNames.top() == QLatin1String("gphoto:id")) {
        //qDebug() << "id" << string << localNames.size() << localNames;
        currentAlbum.id = string;
    }
    return true;
}

bool FeedXmlHandler::endElement ( const QString & namespaceURI, const QString & localName, const QString & qualifiedName )
{
    localNames.pop();
    qualifiedNames.pop();

    if (qualifiedName == QLatin1String("entry")) {
        //qDebug() << "pop";
        feed.albumMetas.append(currentAlbum);
    }

    return true;
}

