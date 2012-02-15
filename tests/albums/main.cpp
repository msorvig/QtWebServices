#include <QtGui/QGuiApplication>
#include <QtCore>

#include "qtgoogleclientlogin.h"
#include "qtpicasaweb.h"
#include "blockingnetworkaccessmanager.h"

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

QByteArray signIn()
{
    QtGoogleClientLogin clientLogin;
    clientLogin.setSettingsKey("tests-album");
    if (!clientLogin.isConfiguredWithSettings()) {
        qDebug() << "User and password required";
        return 0;
        // fill in on first use
        //clientLogin.setLogin("user@gmail.com");
        //clientLogin.setPassword("***********");
    }

    clientLogin.setServiceName("lh2");
    clientLogin.setSourceName("SirWigSoftware-tests-album-0.1");

    AuthenticationState state = clientLogin.authenticate();
    if (state == SuccessfullAuthentication) {
        qDebug() << "login ok" << clientLogin.authenticationToken();
        return clientLogin.authenticationToken();
    } else if (state == FailedAuthentication) {
        qDebug() << "auth failed";
        return QByteArray();
    } else  if (state == CaptchaRequired) {
        qDebug() << "Solve captcha at https://www.google.com/accounts/DisplayUnlockCaptcha";
        return QByteArray();
    }
    return QByteArray();
}

void loadFeedXml(const QByteArray authenticationToken)
{
    QtPicasaWeb picasa;
    picasa.setAuthenticationToken(authenticationToken);
    QByteArray xml = picasa.requestFeed().toUtf8();
    qDebug() << "loadFeedXml got" << xml.count() << "bytes";
    writeFile("data/feed.xml", xml);
}

QtPicasaFeed parseFeed()
{
    QByteArray feedXml = readFile("data/feed.xml");
    qDebug() << "parseFeed read" << feedXml.count() << "bytes";

    QtPicasaWeb picasa;
    QtPicasaFeed feed = picasa.parseFeedXml(feedXml);
    return feed;
}

void loadAlbums(const QByteArray authenticationToken, const QtPicasaFeed &feed)
{
    qDebug() << "loadAlbums" << feed.albumMetas.count();
    QtPicasaWeb picasa;
    picasa.setAuthenticationToken(authenticationToken);

    // load new albums (those with no data file on disk)
    foreach (const QtPicasaAlbumMeta &albumMeta, feed.albumMetas) {
        QString albumFileName = "data/album" + albumMeta.id.toAscii() + ".xml";
        if (QFile(albumFileName).exists())
            continue;

        QByteArray xml = picasa.requestAlbum(albumMeta.id).toUtf8();
        qDebug() << "loadAlbum" << albumMeta.title <<  "got" << xml.count() << "bytes";
        writeFile(albumFileName, xml);
    }

    // load all albums
    foreach (const QtPicasaAlbumMeta &albumMeta, feed.albumMetas) {
        QByteArray xml = picasa.requestAlbum(albumMeta.id).toUtf8();
        qDebug() << "loadAlbum" << albumMeta.title <<  "got" << xml.count() << "bytes";
        QString albumFileName = "data/album" + albumMeta.id.toAscii() + ".xml";
        writeFile(albumFileName, xml);
    }
}

QList<QtPicasaAlbum> parseAlbums()
{
    QList<QtPicasaAlbum> albums;
    QStringList albumFiles = QDir("data").entryList(QStringList() << "album*.xml");
    qDebug() << "parseAlbums files count" << albumFiles.count();

    foreach (const QString &albumFileName, albumFiles) {
        QByteArray albumXml = readFile("data/" + albumFileName);
        qDebug() << "parseAlbum " << albumFileName << "read" << albumXml.count() << "bytes";

        QtPicasaWeb picasa;
        QtPicasaAlbum album = picasa.parseAlbumXml(albumXml);
        qDebug() << "got images" << album.images.count();
        albums.append(album);
    }
    return albums;
}

QSet<QString> seenImageIds; // sanity check;

void loadThumbnails(const QByteArray authenticationToken, QtPicasaAlbum album)
{
    qDebug() << "load thumbnails for" << album.title << album.images.count();
    BlockingNetworkAccessManager BAM;

    QString thumbnailDir = "data/thumbnails/";
    foreach (QtPicasaImage image, album.images) {
        // Continue if there are no thumbnails for this image
        if (image.thumbnails.isEmpty())
            continue;

        QtPicasaThumbnail thumbnail = image.thumbnails.at(0);
        QString thumbNailId = image.id + "-" + QString::number(thumbnail.width) + "-" + QString::number(thumbnail.height);

        // Continue if the file exists
        QFile thumbnailFile(thumbnailDir + thumbNailId + ".jpg"); // #### file type
        if (thumbnailFile.exists()) {
            qDebug() << "skip thumbnail" << image.id << thumbnail.url;
            continue;
        }

        qDebug() << "load thumbnail" << image.id << thumbnail.url;

        QNetworkRequest request(QUrl(thumbnail.url));
        request.setRawHeader("GData-Version", "2");
        request.setRawHeader("Authorization", "GoogleLogin " + authenticationToken);

        QNetworkReply *reply = BAM.syncGet(request);
        reply->deleteLater();
        // ### error handling
        QByteArray data = reply->readAll();
        qDebug() << "got" << data.count() << "bytes";

        // Continue if there was no data
        if (data.count() == 0)
            continue;

        thumbnailFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
        thumbnailFile.write(data);
    }
}

void loadThumbnails(const QByteArray authenticationToken, QList<QtPicasaAlbum> albums)
{
    seenImageIds.clear();
    foreach (QtPicasaAlbum album, albums) {
        loadThumbnails(authenticationToken, album);
    }
}

bool timesort(const QtPicasaImage &a, const QtPicasaImage &b)
{
    return (a.timestamp < b.timestamp);
}

QList<QtPicasaImage> createSortedImageList(QList<QtPicasaAlbum> albums)
{
    QList<QtPicasaImage> images;
    foreach (QtPicasaAlbum album, albums) {
        images.append(album.images);
    }

    qSort(images.begin(), images.end(), timesort);

    return images;
}

void printImageDates(QList<QtPicasaImage> images)
{
    foreach (QtPicasaImage image, images) {
        qDebug() << "image" << image.id << "date" << QDateTime::fromMSecsSinceEpoch(image.timestamp).toString();
    }
}

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QByteArray token = signIn();
    //loadFeedXml(token);

    //QtPicasaFeed feed = parseFeed();
    //qDebug() << "feed" << feed;
    //loadAlbums(token, feed);

    QList<QtPicasaAlbum> albums = parseAlbums();
    //qDebug() << "albums" << albums;
   // loadThumbnails(token, albums);
    QList<QtPicasaImage> images = createSortedImageList(albums);
    qDebug() << "Total image count" << images.count();
    printImageDates(images);


}


