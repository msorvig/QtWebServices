#include "qtpicasaweb.h"
#include "feedxmlparser.h"
#include "albumxmlparser.h"
#include "blockingnetworkaccessmanager.h"
#include <QtCore>


// Headers:
// GData-Version: 2
// Authorization: GoogleLogin auth=yourAuthValue

// Get albums:
// GET https://picasaweb.google.com/data/feed/api/user/userid

// GET https://picasaweb.google.com/data/feed/api/user/userID/albumid/albumID

QtPicasaWeb::QtPicasaWeb(QObject *parent) :
    QObject(parent)
{
    m_networkAccessManager = new BlockingNetworkAccessManager(this);
}

QtPicasaWeb::~QtPicasaWeb()
{
    delete m_networkAccessManager;
}

void QtPicasaWeb::setAuthenticationToken(const QByteArray &authenticationToken)
{
    m_authenticationToken = authenticationToken;
}

QString QtPicasaWeb::requestFeed()
{
    QNetworkRequest request(QUrl("https://picasaweb.google.com/data/feed/api/user/default"));
    request.setRawHeader("GData-Version", "2");
    request.setRawHeader("Authorization", "GoogleLogin " + m_authenticationToken.toLatin1());
    QNetworkReply *reply = m_networkAccessManager->syncGet(request);
    reply->deleteLater();
    // ### error handling
    return reply->readAll();
}

QString QtPicasaWeb::requestAlbum(const QString &albumId)
{
    QNetworkRequest request(QUrl("https://picasaweb.google.com/data/feed/api/user/default/albumid/" + albumId.toLatin1()));
    request.setRawHeader("GData-Version", "2");
    request.setRawHeader("Authorization", "GoogleLogin " + m_authenticationToken.toLatin1());
    QNetworkReply *reply = m_networkAccessManager->syncGet(request);
    reply->deleteLater();
    // ### error handling
    return reply->readAll();
}

QtPicasaFeed QtPicasaWeb::parseFeedXml(const QByteArray &xml)
{
    FeedXmlHandler parser;
    return parser.parseFeedXml(xml);
}

QtPicasaAlbum QtPicasaWeb::parseAlbumXml(const QByteArray &xml)
{
    AlbumXmlHandler parser;
    return parser.parseAlbumXml(xml);
}
