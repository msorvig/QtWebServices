#ifndef QTPICASALOADER_H
#define QTPICASALOADER_H

#include "picasatypes.h"
#include "qtpicasaweb.h"
#include <QtCore/QString>

//
//  QtPicasaLoader downloads picasa album information and
//  stores it on disk in storagePath. Functions with the
//  "download" prefix accesses the network and requires
//   signing in with signIn() first.
//
//  Functions with the "parse" prefix loads data from disk
//  and does not use the network.
//
//  All functions are synchronous. The class is thread-
//  safe.
//
class QtPicasaLoader
{
public:
    QtPicasaLoader(const QString &storagePath);

    void setAccessToken(const QString &accessToken);

    void downloadFeedXml();
    QtPicasaFeed parseFeed();
    void saveFeed(const QtPicasaFeed &feed);
    QtPicasaFeed loadFeed();

    void downloadAlbums(const QtPicasaFeed &feed);
    QList<QtPicasaAlbum> parseAlbums();

    void saveAlbum(const QtPicasaAlbum &album);
    QtPicasaAlbum loadAlbum(const QString &fileName) const;
    QList<QtPicasaAlbum> loadAlbums() const;

    void downloadThumbnails(const QtPicasaAlbum &album);
    void downloadThumbnails(const QList<QtPicasaAlbum> &albums);
    QString thumbnailDiskPath(const QtPicasaImage &image, int thumbnailIndex);

    QString imageDiskPath(const QtPicasaImage &image);
private:
    QString m_storagePath;
    QString m_accessToken;

    QtPicasaWeb m_picasa;
};

#endif
