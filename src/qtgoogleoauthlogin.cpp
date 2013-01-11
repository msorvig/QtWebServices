#include "qtgoogleoauthlogin.h"


QtGoogleOAuthLogin::QtGoogleOAuthLogin(QObject *parent)
    :QObject(parent)
    ,m_oauthUrl(QStringLiteral("https://accounts.google.com/o/oauth2/auth"))
    ,m_tokenUrl(QStringLiteral("https://accounts.google.com/o/oauth2/token"))
    ,m_responseType(QStringLiteral("code"))
    ,m_redirectUri(QStringLiteral("urn:ietf:wg:oauth:2.0:oob")) // return access code in web page title
    ,m_networkAccessManager(new QNetworkAccessManager)
{
    connect(m_networkAccessManager, SIGNAL(finished(QNetworkReply*)), SLOT(replyFinished(QNetworkReply*)));
}

QtGoogleOAuthLogin::~QtGoogleOAuthLogin()
{

}

void QtGoogleOAuthLogin::setClientId(const QString &id)
{
    m_clientId = id;
}

QString QtGoogleOAuthLogin::clientId() const
{
    return m_clientId;
}

void QtGoogleOAuthLogin::setClientSecret(const QString &secret)
{
    m_clientSecret = secret;
}

QString QtGoogleOAuthLogin::clientSecret() const
{
    return m_clientSecret;
}

void QtGoogleOAuthLogin::setScope(const QString &scope)
{
    m_scope = scope;
}

QString QtGoogleOAuthLogin::scope() const
{
    return m_scope;
}

void QtGoogleOAuthLogin::setRefreshToken(const QString &refreshToken)
{
    m_refreshToken = refreshToken;
}

QString QtGoogleOAuthLogin::refreshToken() const
{
    return m_refreshToken;
}

QString QtGoogleOAuthLogin::accessToken() const
{
    return m_accessToken;
}

/*
    Returns the Google OAuth login url.
*/
QUrl QtGoogleOAuthLogin::authorizationUrl() const
{
    QUrl oauth2Url(m_oauthUrl);
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("response_type"), m_responseType);
    query.addQueryItem(QStringLiteral("client_id"), m_clientId);
    query.addQueryItem(QStringLiteral("redirect_uri"), m_redirectUri);
    query.addQueryItem(QStringLiteral("scope"), m_scope);
    query.addQueryItem(QStringLiteral("state"), QStringLiteral("authenticae"));
    oauth2Url.setQuery(query);
    return oauth2Url;
}

QtGoogleOAuthLogin::AuthorizationResult QtGoogleOAuthLogin::parseAuthorizationResponse(const QString &webPageTitle)
{
    if (webPageTitle.startsWith(QStringLiteral("Success")))
        return QtGoogleOAuthLogin::AuthorizationGranted;
    else if (webPageTitle.startsWith(QStringLiteral("Denied")))
        return QtGoogleOAuthLogin::AuthorizationDenied;

    // Error case:
    return QtGoogleOAuthLogin::AuthorizationError;
}

QString QtGoogleOAuthLogin::getAuthorizationCodeFromWebPageTitle(const QString &webPageTitle)
{
    // Success looks like "Success state=authenticae&code=4/HXhBrAxb9h20ZYnSsiz_KckF2_Zw"
    // Extract the code part after '='
    if (!webPageTitle.startsWith(QStringLiteral("Success")))
        return QString();

    int codeIndex = webPageTitle.indexOf(QStringLiteral("code"));
    if (codeIndex == -1)
        return QString();

    QString code = webPageTitle.mid(codeIndex);
    QStringList parts = code.split(QStringLiteral("="));
    if (!parts.count() == 2)
        return QString();

    return parts.at(1);
}

void QtGoogleOAuthLogin::initiateAccess()
{
    if (m_refreshToken.isEmpty()) {
        emit displayLoginPage(authorizationUrl());
    } else {
        initiaAccessFromRefreshToken(m_refreshToken);
    }
}

void QtGoogleOAuthLogin::initiateAccessFromLoginWebPageTitle(const QString &webPageTitle)
{
    QtGoogleOAuthLogin::AuthorizationResult result = parseAuthorizationResponse(webPageTitle);
    if (result == QtGoogleOAuthLogin::AuthorizationGranted) {
        qDebug() << "AuthorizationGranted";
        QString authorizationCode = getAuthorizationCodeFromWebPageTitle(webPageTitle);
        if (!authorizationCode.isEmpty()) {
            initiateAccessFromAuthorizationCode(authorizationCode);
        } else {
            error(QStringLiteral("unexpected"));
        }
    } else if (result == QtGoogleOAuthLogin::AuthorizationDenied) {
        error(QStringLiteral("Access denied"));
    } else {
        error(QStringLiteral("unexpected"));
    }
}

void QtGoogleOAuthLogin::initiateAccessFromAuthorizationCode(const QString &authorizationCode)
{
    QNetworkRequest request(m_tokenUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");

    QUrlQuery query;
    query.addQueryItem(QStringLiteral("code"), authorizationCode);
    query.addQueryItem(QStringLiteral("client_id"), m_clientId);
    query.addQueryItem(QStringLiteral("client_secret"), m_clientSecret);
    query.addQueryItem(QStringLiteral("redirect_uri"), m_redirectUri); // ### ??
    query.addQueryItem(QStringLiteral("grant_type"), QStringLiteral("authorization_code"));

    postNetworkRequest(request, query.toString(QUrl::FullyEncoded).toLatin1());
}

void QtGoogleOAuthLogin::initiaAccessFromRefreshToken(const QString &refreshToken)
{
    QNetworkRequest request(m_tokenUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");

    QUrlQuery query;
    query.addQueryItem(QStringLiteral("refresh_token"), refreshToken);
    query.addQueryItem(QStringLiteral("client_id"), m_clientId);
    query.addQueryItem(QStringLiteral("client_secret"), m_clientSecret);
    query.addQueryItem(QStringLiteral("grant_type"), QStringLiteral("refresh_token"));

    postNetworkRequest(request, query.toString(QUrl::FullyEncoded).toLatin1());
}

void QtGoogleOAuthLogin::postNetworkRequest(const QNetworkRequest &request, const QByteArray &data)
{
    m_reply = m_networkAccessManager->post(request, data);
    connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(slotError(QNetworkReply::NetworkError)));
    connect(m_reply, SIGNAL(sslErrors(QList<QSslError>)),
            this, SLOT(slotSslErrors(QList<QSslError>)));
}

void QtGoogleOAuthLogin::replyFinished(QNetworkReply *reply)
{
    reply->deleteLater();
    QByteArray replyContents = reply->readAll();

    QJsonDocument doc = QJsonDocument::fromJson(replyContents);
    if (!doc.isObject()) {
        //emit error();
        qDebug() << "error";
        return;
    }

    QJsonObject object = doc.object();
    if (object.contains(QStringLiteral("access_token"))) {
        m_accessToken = object.value(QStringLiteral("access_token")).toString();
        emit accessTokenReady(m_accessToken);
    }

    if (object.contains(QStringLiteral("refresh_token"))) {
        m_refreshToken = object.value(QStringLiteral("refresh_token")).toString();
        emit refreshTokenReady(m_refreshToken);
    }
}

void QtGoogleOAuthLogin::slotError(QNetworkReply::NetworkError error)
{
    QByteArray replyContents = m_reply->readAll();
    qDebug() << "error" << replyContents;
    //emit error(replyContents);
}

void QtGoogleOAuthLogin::slotSslErrors(QList<QSslError> sslErrors)
{
    m_reply->ignoreSslErrors();
}





