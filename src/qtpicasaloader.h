#ifndef QTPICASALOADER_H
#define QTPICASALOADER_H

#include "picasatypes.h"
#include "qtgoogleclientlogin.h"
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

    void setApplicatonAPISourceName(const QString &sourceName);
    void setSettingsKey(const QString &settingsKey);

    AuthenticationState signIn(const QString &user, const QString &pass);
    bool hasSignInCache();
    AuthenticationState signInCached();

    void downloadFeedXml();
    QtPicasaFeed parseFeed();

    void downloadAlbums(const QtPicasaFeed &feed);
    QList<QtPicasaAlbum> parseAlbums();

    void downloadThumbnails(const QtPicasaAlbum &album);
    void downloadThumbnails(const QList<QtPicasaAlbum> &albums);
    QString thumbnailDiskPath(const QtPicasaImage &image, int thumbnailIndex);

    QString imageDiskPath(const QtPicasaImage &image);
private:
    QString m_storagePath;
    QString m_appSourceName;

    QtGoogleClientLogin m_clientLogin;
    QtPicasaWeb m_picasa;
};

#endif
