#ifndef QTPICASAWEB_H
#define QTPICASAWEB_H

#include <QObject>
#include <QtCore>

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

class QtPicasaWeb : public QObject
{
    Q_OBJECT
public:
    explicit QtPicasaWeb(QObject *parent = 0);

    // Log in first using QtGoogleClientLogin, get the authentication
    // token and pass that to this function.
    void setAuthenticationToken(const QByteArray &authenticationToken);

    // Parsed data accessors, caches
    QList<QtPicasaAlbum> getAlbumList();
    QByteArray getPictureList();

    // raw accessors, always goes to the network
    QByteArray getAlbumXml();
    QByteArray getImageXml(const QByteArray &albumId);

    // xml parsing
    QtPicasaAlbum parseAlbumXml(const QByteArray &xmldata);


signals:

public slots:

};

#endif // QTPICASAWEB_H
