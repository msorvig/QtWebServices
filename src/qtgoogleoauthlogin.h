#ifndef QTGOOGLEOAUTHLOGIN_H
#define QTGOOGLEOAUTHLOGIN_H

#include <QtCore/QtCore>
#include <QtNetwork>

class QtGoogleOAuthLogin : public QObject
{
    Q_OBJECT
public:
    explicit QtGoogleOAuthLogin(QObject *parent = 0);
    ~QtGoogleOAuthLogin();

    // Application configuration setters
    void setClientId(const QString &id);
    QString clientId() const;
    void setClientSecret(const QString &secret);
    QString clientSecret() const;
    void setScope(const QString &scope);
    QString scope() const;

    // The refresh token is acquired at first authorization
    // and can be saved for later use.
    void setRefreshToken(const QString &refreshToken);
    QString refreshToken() const;

    // The access token is acuired at quthorization and is
    // use with other APIs. It will expire, in which case
    // the refresh token can be used to gain a new access
    // token.
    QString accessToken() const;

    // Authorization is browser-based and external to this class.
    QUrl authorizationUrl() const;

    // Functions for initiation authorization and getting access tokens.
    void initiateAccess();
    void initiateAccessFromLoginWebPageTitle(const QString &webPageTitle);
    void initiateAccessFromAuthorizationCode(const QString &authorizationCode);
    void initiaAccessFromRefreshToken(const QString &refreshToken);

Q_SIGNALS:
    // Callouts: Users of this class should connect to these
    void displayLoginPage(const QUrl &authorizationUrl);
    void refreshTokenReady(const QString &refreshToken);
    void accessTokenReady(const QString &refreshToken);
    void error(const QString &error);

private:
    enum AuthorizationResult {
         AuthorizationGranted,
         AuthorizationDenied,
         AuthorizationError
    };
    AuthorizationResult parseAuthorizationResponse(const QString &webPageTitle);
    QString getAuthorizationCodeFromWebPageTitle(const QString &webPageTitle);

    void postNetworkRequest(const QNetworkRequest &request, const QByteArray &data);

private Q_SLOTS:
    void replyFinished(QNetworkReply *reply);
    void slotError(QNetworkReply::NetworkError error);
    void slotSslErrors(QList<QSslError> sslErrors);

private:
    // protocol constants
    QString m_oauthUrl;
    QString m_tokenUrl;
    QString m_responseType;
    QString m_redirectUri;

    // per-applicaiton constants
    QString m_clientId;
    QString m_clientSecret;
    QString m_scope;

    // run-time variables
    QString m_accessToken;
    QString m_refreshToken;

    // network access
    QNetworkAccessManager *m_networkAccessManager;
    QNetworkReply *m_reply;
};

#endif
