#ifndef QTPICASAWEB_H
#define QTPICASAWEB_H

#include "picasatypes.h"

#include <QObject>
#include <QtCore>
#include <QtNetwork>
#include <QSslError>
#include <QSslSocket>


class BlockingNetworkAccessManager;
class QtPicasaWeb : public QObject
{
    Q_OBJECT
public:
    explicit QtPicasaWeb(QObject *parent = 0);
    ~QtPicasaWeb();

    // Log in first using QtGoogleClientLogin, get the authentication
    // token and pass that to this function.
    void setAccessToken(const QString &accessToken);

    QByteArray requestFeed();
    QByteArray requestAlbum(const QString &albumId);
    QByteArray requestThumbnail(const QString &url);

    // xml parsing
    QtPicasaFeed parseFeedXml(const QByteArray &xmldata);
    QtPicasaAlbum parseAlbumXml(const QByteArray &xmldata);
signals:
    void readyFeedRequest();
    void readyAlbumRequest();

private:
    QString m_accessToken;
    BlockingNetworkAccessManager *m_networkAccessManager;
    QString m_replyContents;
};


#endif // QTPICASAWEB_H
