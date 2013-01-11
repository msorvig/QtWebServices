#include "applogic.h"
#include "qtpicasaweb.h"
#include "qtpicasaloader.h"
#include <QtWidgets>

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

AppLogic::AppLogic()
{
    login = new QtGoogleOAuthLogin(this);
    login->setClientId(QStringLiteral("**********.apps.googleusercontent.com"));
    login->setClientSecret(QStringLiteral("********************"));
    login->setScope(QStringLiteral("http://picasaweb.google.com/data/"));

    QSettings settings;
    login->setRefreshToken(settings.value("refreshToken").toString());

    connect(login, SIGNAL(displayLoginPage(QUrl)), SLOT(displayLoginPage(QUrl)));
    connect(login, SIGNAL(accessTokenReady(QString)), SLOT(accessTokenReady(QString)));
    connect(login, SIGNAL(refreshTokenReady(QString)), SLOT(refreshTokenReady(QString)));
    connect(login, SIGNAL(error(QString)), SLOT(error(QString)));
}

void AppLogic::start()
{
    login->initiateAccess();
}

void AppLogic::displayLoginPage(const QUrl &rul)
{
    loginWidget = new QtGoogleLoginWidget();
    loginWidget->move(50, 50);
    connect(loginWidget, SIGNAL(pageTitleChanged(QString)), SLOT(handleLoginPageLoad(QString)));
    loginWidget->displayLoginPage(login->authorizationUrl());
}

void AppLogic::handleLoginPageLoad(const QString &webPageTitle)
{
    loginWidget->hide();
    delete loginWidget;
    login->initiateAccessFromLoginWebPageTitle(webPageTitle);
}

void AppLogic::accessTokenReady(const QString &accessToken)
{
    qDebug() << "accessTokenReady" << accessToken;

    QtPicasaLoader loader("data");
    loader.setAccessToken(accessToken);

    loader.downloadFeedXml();
    QtPicasaFeed feed = loader.parseFeed();

    loader.downloadAlbums(feed);
    QList<QtPicasaAlbum> albums = loader.parseAlbums();
    //loader.saveAlbums(albums);
    //QList<QtPicasaAlbum> albums = loader.loadAlbums();
    loader.downloadThumbnails(albums);

    QList<QtPicasaImage> images = createSortedImageList(albums);
    qDebug() << "Total image count" << images.count();
    printImageDates(images);
}

void AppLogic::refreshTokenReady(const QString &refreshToken)
{
    qDebug() << "refreshTokenReady" << refreshToken;
    QSettings settings;
    settings.setValue("refreshToken", refreshToken);
}

void AppLogic::error(const QString &error)
{
    qDebug() << "error error error" << error;
}


