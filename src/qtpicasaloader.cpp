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
    QDir().mkpath(m_storagePath);
    qDebug() << "QtPicasaLoader storage path" << QDir(m_storagePath).absolutePath();
}

void QtPicasaLoader::setAccessToken(const QString &accessToken)
{
    m_accessToken = accessToken;
    m_picasa.setAccessToken(accessToken);
}

void QtPicasaLoader::downloadFeedXml()
{
    QByteArray xml = m_picasa.requestFeed();
    qDebug() << "loadFeedXml got" << xml.count() << "bytes";
    writeFile(m_storagePath + "/feed.xml", xml);
}

QtPicasaFeed QtPicasaLoader::parseFeed()
{
    QByteArray feedXml = readFile(m_storagePath + "/feed.xml");
    qDebug() << "parseFeed read" << feedXml.count() << "bytes";

    return m_picasa.parseFeedXml(feedXml);
}

void QtPicasaLoader::saveFeed(const QtPicasaFeed &feed)
{
    QByteArray data;
    QBuffer buff(&data);
    QDataStream stream(&buff);
    stream << feed;

    QString fileName = m_storagePath + "/feed.bin";
    writeFile(fileName, data);
}

QtPicasaFeed QtPicasaLoader::loadFeed()
{
    QByteArray data = readFile(m_storagePath + "/feed.bin");
    QBuffer buff(&data);
    QDataStream stream(&buff);

    QtPicasaFeed feed;
    stream >> feed;
    return feed;
}

void QtPicasaLoader::downloadAlbums(const QtPicasaFeed &feed)
{
    qDebug() << "loadAlbums" << feed.albumMetas.count();
    // load new albums (those with no data file on disk)
    foreach (const QtPicasaAlbumMeta &albumMeta, feed.albumMetas) {
        QString albumFileName = m_storagePath +"/album" + albumMeta.id.toLatin1() + ".xml";
        if (QFile(albumFileName).exists())
            continue;

        QByteArray xml = m_picasa.requestAlbum(albumMeta.id);
        qDebug() << "loadAlbum" << albumMeta.title <<  "got" << xml.count() << "bytes";
        writeFile(albumFileName, xml);
    }

    // load all albums
    foreach (const QtPicasaAlbumMeta &albumMeta, feed.albumMetas) {
        QByteArray xml = m_picasa.requestAlbum(albumMeta.id);
        qDebug() << "loadAlbum" << albumMeta.title <<  "got" << xml.count() << "bytes";
        QString albumFileName = m_storagePath +"/album" + albumMeta.id.toLatin1() + ".xml";
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

void QtPicasaLoader::saveAlbum(const QtPicasaAlbum &album)
{
    QByteArray data;
    QBuffer buff(&data);
    QDataStream stream(&buff);
    stream << album;

    QString fileName = m_storagePath + "/album" + album.title + "bin";
    writeFile(fileName, data);
}

QtPicasaAlbum QtPicasaLoader::loadAlbum(const QString &fileName) const
{
    QByteArray data = readFile(fileName);
    QBuffer buff(&data);
    QDataStream stream(&buff);

    QtPicasaAlbum album;
    stream >> album;
    return album;
}

QList<QtPicasaAlbum> QtPicasaLoader::loadAlbums() const
{
/*
    QList<QtPicasaAlbum> albums;
    QStringList albumFiles = QDir(m_storagePath).entryList(QStringList() << "album-bin*");
    foreach (QString albumFiles, albumFiles) {
        album
    }
*/
}


void QtPicasaLoader::downloadThumbnails(const QtPicasaAlbum &album)
{
    qDebug() << "load thumbnails for" << album.title << album.images.count();
    foreach (QtPicasaImage image, album.images) {
        // Continue if there are no thumbnails for this image
        if (image.thumbnails.isEmpty())
            continue;

        const int thumbnailIndex = 0;
        QtPicasaThumbnail thumbnail = image.thumbnails.at(thumbnailIndex);
        QString thumbnailPath = thumbnailDiskPath(image, thumbnailIndex);

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

QString QtPicasaLoader::thumbnailDiskPath(const QtPicasaImage &image, int thumbnailIndex)
{
    if (thumbnailIndex < 0 || thumbnailIndex >= image.thumbnails.count())
        return QString();
    QtPicasaThumbnail thumbnail = image.thumbnails.at(thumbnailIndex);
    QString thumbNailId = image.id + "-" + QString::number(thumbnail.width) + "-" + QString::number(thumbnail.height);
    return m_storagePath + "/thumbnails/" + thumbNailId + ".jpg"; // #### file type
}

