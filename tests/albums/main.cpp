#include <QtGui/QGuiApplication>
#include <QtCore>

#include "qtgoogleclientlogin.h"
#include "qtpicasaweb.h"

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
        qDebug() << "parseFeed read" << albumXml.count() << "bytes";

        QtPicasaWeb picasa;
        QtPicasaAlbum album = picasa.parseAlbumXml(albumXml);
        albums.append(album);
    }
    return albums;
}


int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QByteArray token = signIn();
    loadFeedXml(token);

    QtPicasaFeed feed = parseFeed();
    //qDebug() << "feed" << feed;
    loadAlbums(token, feed);

    QList<QtPicasaAlbum> albums = parseAlbums();
    //qDebug() << "albums" << albums;
}


