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
    void setAuthenticationToken(const QByteArray &authenticationToken);

    QString requestFeed();
    QString requestAlbum(const QString &albumId);
    QByteArray requestThumbnail(const QString &url);

    // xml parsing
    QtPicasaFeed parseFeedXml(const QByteArray &xmldata);
    QtPicasaAlbum parseAlbumXml(const QByteArray &xmldata);
signals:
    void readyFeedRequest();
    void readyAlbumRequest();

private:
    QString m_authenticationToken;
    BlockingNetworkAccessManager *m_networkAccessManager;
    QString m_replyContents;
};


#endif // QTPICASAWEB_H
