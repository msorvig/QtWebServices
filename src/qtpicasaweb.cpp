#include "qtpicasaweb.h"
#include <QtCore>
#include <QtXml>

// Headers:
// GData-Version: 2
// Authorization: GoogleLogin auth=yourAuthValue

// Get albums:
// GET https://picasaweb.google.com/data/feed/api/user/userid

// GET https://picasaweb.google.com/data/feed/api/user/userID/albumid/albumID

QtPicasaWeb::QtPicasaWeb(QObject *parent) :
    QObject(parent)
{
}

class AlbumXmlHandler : public QXmlDefaultHandler
{
public:
    QtPicasaAlbum album;
    QtPicasaImage currentImage;
    enum Mode { Base, AlbumTitle, Group, ImageTitle, ImageDescription };
    QStack<Mode> mode;

    AlbumXmlHandler()
        :QXmlDefaultHandler()
    {
        mode.push(Base);
    }

    bool fatalError (const QXmlParseException & exception)
    {
        qWarning() << "Fatal error on line" << exception.lineNumber()
                   << ", column" << exception.columnNumber() << ":"
                   << exception.message() << exception.publicId();

        return true;
    }

    bool startElement ( const QString & namespaceURI, const QString & localName, const QString & qName, const QXmlAttributes & atts )
    {
        if (localName == "group") {
            //qDebug() << "start element" << localName << atts.count();
            mode.push(Group);
            currentImage = QtPicasaImage();
        }

        if (mode.top() == Group) {
            if (localName == "title") {
                mode.push(ImageTitle);
            }
            if (localName == "description") {
                mode.push(ImageDescription);
            }
            if (localName == "content") {
                currentImage.url = atts.value("url");
            }
            if (localName == "thumbnail") {
                currentImage.thumbnailUrl = atts.value("url");
            }
        }
        return true;
    }

    bool characters ( const QString & string )
    {
        if (mode.top() == ImageTitle) {
            currentImage.title = string;
        }
        if (mode.top() == ImageDescription) {
            currentImage.description = string;
        }
        return true;
    }

    bool endElement ( const QString & namespaceURI, const QString & localName, const QString & qName )
    {
        if (localName == "group") {
            //qDebug() << "end element" << localName;
            mode.pop();

            qDebug() << "Add image" << currentImage.title << currentImage.description
                                    << currentImage.url << currentImage.thumbnailUrl;

            album.images.append(currentImage);
        }

        if (mode.top() == ImageTitle && localName == "title") {
            mode.pop();
        }
        if (mode.top() == ImageDescription && localName == "description") {
            mode.pop();
        }

        return true;
    }

};

QtPicasaAlbum QtPicasaWeb::parseAlbumXml(const QByteArray &xml)
{
    QtPicasaAlbum album;
    qDebug() << "QtPicasaWeb::parseAlbumXm got" << xml.count() << "bytes";


    QBuffer xmlBuffer;
    xmlBuffer.setData(xml);
    QXmlSimpleReader xmlReader;
    QXmlInputSource *source = new QXmlInputSource(&xmlBuffer);
    AlbumXmlHandler *handler = new AlbumXmlHandler();
    xmlReader.setContentHandler(handler);
    xmlReader.setErrorHandler(handler);

    bool ok = xmlReader.parse(source);
    qDebug() << "xml parse ok" << ok;

    return album;
}
