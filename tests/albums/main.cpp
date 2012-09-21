#include <QtGui/QGuiApplication>
#include <QtCore>

#include "qtgoogleclientlogin.h"
#include "qtpicasaloader.h"

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

    QtPicasaLoader loader("data");
    loader.setApplicatonAPISourceName("SirWigSoftware-tests-album-0.1");
    loader.setSettingsKey("tests-album");

    if (loader.hasSignInCache()) {
        loader.signInCached();
    } else {
        qFatal("User and password required");
        loader.signIn("user@gmail.com", "***********");
    }

    loader.downloadFeedXml();
    QtPicasaFeed feed = loader.parseFeed();

    loader.downloadAlbums(feed);
    QList<QtPicasaAlbum> albums = loader.parseAlbums();
    loader.saveAlbums(albums);
    QList<QtPicasaAlbum> albums = loader.loadAlbums();
    loader.downloadThumbnails(albums);

    QList<QtPicasaImage> images = createSortedImageList(albums);
    qDebug() << "Total image count" << images.count();
    printImageDates(images);
}


