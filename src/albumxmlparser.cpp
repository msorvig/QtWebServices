#include "albumxmlparser.h"


AlbumXmlHandler::AlbumXmlHandler()
{

}

QtPicasaAlbum AlbumXmlHandler::parseAlbumXml(const QByteArray &xml)
{
    addData(xml);

    while (readNextStartElement()) {
        //qDebug() << "element" << name() << qualifiedName();
        if (name() == "feed")
            parseFeed();
        else
            skipCurrentElement();
    }
    return album;
}

void AlbumXmlHandler::parseFeed()
{
    while (readNextStartElement()) {
        //qDebug() << "feed element" << name() << qualifiedName();
        if (name() == "entry")
            parseEntry();
        else
            skipCurrentElement();
    }
}

void AlbumXmlHandler::parseEntry()
{
    QtPicasaImage image;
    //qDebug() << " ";
    while (readNextStartElement()) {
    //   qDebug() << "entry element" << name() << qualifiedName();
       if (qualifiedName() == "media:group") {
            parseMediaGroup(&image);
       } else if (name() == "title") {
            image.title = readElementText();
            //qDebug() << "title" << image.title;
       } else if (qualifiedName() == "gphoto:id") {
            image.id = readElementText();
       } else if (qualifiedName() == "gphoto:albumid") {
            image.albumid = readElementText();
       } else if (qualifiedName() == "gphoto:width") {
            image.width = readElementText().toInt();
       } else if (qualifiedName() == "gphoto:height") {
            image.height = readElementText().toInt();
       } else if (qualifiedName() == "gphoto:size") {
            image.size = readElementText().toInt();
       } else if (qualifiedName() == "gphoto:timestamp") {
            image.timestamp = readElementText().toLongLong();
       } else {
            skipCurrentElement();
       }
    }
    //qDebug() << "append image";
    album.images.append(image);
}

void AlbumXmlHandler::parseLink()
{
    while (readNextStartElement()) {
        qDebug() << "link element" << name() << qualifiedName();
         skipCurrentElement();
    }
}

void AlbumXmlHandler::parseMediaGroup(QtPicasaImage *image)
{
    while (readNextStartElement()) {
        if (0) {
#if 0
        } else if (qualifiedName() == "media:content") {
         //   QString text = readElementText();
         //   qDebug() << "media:content" << text;
        } else if (qualifiedName() == "media:credit") {
            QString text = readElementText();
         //   qDebug() << "media:credit" << text;
        } else if (qualifiedName() == "media:description") {
            QString text = readElementText();
         //   qDebug() << "media:description" << text;
        } else if (qualifiedName() == "media:keywords") {
            QString text = readElementText();
         //   qDebug() << "media:keywords" << text;
#endif
        } else if (qualifiedName() == "media:thumbnail") {
            parseThumbnail(image);
//        } else if (qualifiedName() == "media:title") {
//            QString text = readElementText();
         //   qDebug() << "media:title" << text;
        } else {
            skipCurrentElement();
        }
    }
}

void AlbumXmlHandler::parseThumbnail(QtPicasaImage *image)
{
    QtPicasaThumbnail thumnail;
    foreach (QXmlStreamAttribute att, attributes()) {
        if (0) {
        } else if (att.name() == "width") {
            thumnail.width = att.value().toString().toInt();
        } else if (att.name() == "height") {
            thumnail.height = att.value().toString().toInt();
        } else if (att.name() == "url") {
            thumnail.url = att.value().toString();
        }
    }
    skipCurrentElement();
    image->thumbnails.append(thumnail);
}

