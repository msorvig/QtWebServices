#include "qtpicasaloader.h"

void writeFile(const QString &fileName, const QByteArray &data)
{
    QByteArray compresed = qCompress(data);

    QFile feedFile(fileName);
    feedFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
    feedFile.write(compresed);
}

QByteArray readFile(const QString &fileName)
{
    QFile feedFile(fileName);
    feedFile.open(QIODevice::ReadOnly);
    QByteArray feedXml = feedFile.readAll();
    return qUncompress(feedXml);
}

QtPicasaLoader::QtPicasaLoader(const QString &storagePath)
    :m_storagePath(storagePath)
{
    m_clientLogin.setServiceName("lh2"); // picasa
}

void QtPicasaLoader::setApplicatonAPISourceName(const QString &sourceName)
{
    m_clientLogin.setSourceName(sourceName);
}

void QtPicasaLoader::setSettingsKey(const QString &settingsKey)
{
    m_clientLogin.setSettingsKey(settingsKey);
}

AuthenticationState QtPicasaLoader::signIn(const QString &user, const QString &pass)
{
    m_clientLogin.setLogin(user);
    m_clientLogin.setPassword(pass);
    return signInCached();
}

bool QtPicasaLoader::hasSignInCache()
{
    return m_clientLogin.isConfiguredWithSettings();
}

AuthenticationState QtPicasaLoader::signInCached()
{
    QtGoogleClientLogin clientLogin;
    if (!clientLogin.isConfiguredWithSettings()) {
        qDebug() << "User and password required";
        return NoAuthentication;
    }

    return m_clientLogin.authenticate();
}

void QtPicasaLoader::downloadFeedXml()
{
    QtPicasaWeb picasa;
    picasa.setAuthenticationToken(m_clientLogin.authenticationToken());
    QByteArray xml = picasa.requestFeed().toUtf8();
    qDebug() << "loadFeedXml got" << xml.count() << "bytes";
    writeFile(m_storagePath + "/feed.xml", xml);
}

QtPicasaFeed QtPicasaLoader::parseFeed()
{
    QByteArray feedXml = readFile(m_storagePath + "/feed.xml");
    qDebug() << "parseFeed read" << feedXml.count() << "bytes";

    return m_picasa.parseFeedXml(feedXml);
}

void QtPicasaLoader::downloadAlbums(const QtPicasaFeed &feed)
{
    qDebug() << "loadAlbums" << feed.albumMetas.count();
    // load new albums (those with no data file on disk)
    foreach (const QtPicasaAlbumMeta &albumMeta, feed.albumMetas) {
        QString albumFileName = m_storagePath +"/album" + albumMeta.id.toAscii() + ".xml";
        if (QFile(albumFileName).exists())
            continue;

        QByteArray xml = m_picasa.requestAlbum(albumMeta.id).toUtf8();
        qDebug() << "loadAlbum" << albumMeta.title <<  "got" << xml.count() << "bytes";
        writeFile(albumFileName, xml);
    }

    // load all albums
    foreach (const QtPicasaAlbumMeta &albumMeta, feed.albumMetas) {
        QByteArray xml = m_picasa.requestAlbum(albumMeta.id).toUtf8();
        qDebug() << "loadAlbum" << albumMeta.title <<  "got" << xml.count() << "bytes";
        QString albumFileName = m_storagePath +"/album" + albumMeta.id.toAscii() + ".xml";
        writeFile(albumFileName, xml);
    }
}

QList<QtPicasaAlbum> QtPicasaLoader::parseAlbums()
{
    QList<QtPicasaAlbum> albums;
    QStringList albumFiles = QDir(m_storagePath).entryList(QStringList() << "album*.xml");
    qDebug() << "parseAlbums files count" << albumFiles.count();

    foreach (const QString &albumFileName, albumFiles) {
        QByteArray albumXml = readFile(m_storagePath + "/" + albumFileName);
        qDebug() << "parseAlbum " << albumFileName << "read" << albumXml.count() << "bytes";

        QtPicasaAlbum album = m_picasa.parseAlbumXml(albumXml);
        qDebug() << "got images" << album.images.count();
        albums.append(album);
    }
    return albums;
}

void QtPicasaLoader::downloadThumbnails(const QtPicasaAlbum &album)
{
    qDebug() << "load thumbnails for" << album.title << album.images.count();
    foreach (QtPicasaImage image, album.images) {
        // Continue if there are no thumbnails for this image
        if (image.thumbnails.isEmpty())
            continue;

        QtPicasaThumbnail thumbnail = image.thumbnails.at(0);
        QString thumbnailPath = thumbnailDiskPath(thumbnail);

        // Continue if the file exists
        QFile thumbnailFile(thumbnailPath);
        if (thumbnailFile.exists()) {
            qDebug() << "skip thumbnail" << image.id << thumbnail.url;
            continue;
        }

        qDebug() << "load thumbnail" << image.id << thumbnail.url;
        QByteArray data = m_picasa.requestThumbnail(thumbnail.url);
        // Continue if there was no data
        if (data.count() == 0)
            continue;

        thumbnailFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
        thumbnailFile.write(data);
    }
}

void QtPicasaLoader::downloadThumbnails(const QList<QtPicasaAlbum> &albums)
{
    foreach (QtPicasaAlbum album, albums) {
        downloadThumbnails(album);
    }
}

QString QtPicasaLoader::imageDiskPath(const QtPicasaImage &image)
{
    return m_storagePath + "/images/" + image.id + ".jpg"; // #### file type
}

QString QtPicasaLoader::thumbnailDiskPath(const QtPicasaThumbnail &thumbnail)
{
    QString thumbNailId = image.id + "-" + QString::number(thumbnail.width) + "-" + QString::number(thumbnail.height);
    return m_storagePath + "/thumbnails/" + thumbNailId + ".jpg"; // #### file type
}

