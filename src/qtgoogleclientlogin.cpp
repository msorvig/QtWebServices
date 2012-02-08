#include "qtgoogleclientlogin.h"
#include <QtNetwork>
#include <QSslError>
#include <QSslSocket>

QtGoogleClientLogin::QtGoogleClientLogin(QObject *parent)
    : QObject(parent)
{
    m_networkAccessManager = new QNetworkAccessManager(this);
    connect(m_networkAccessManager, SIGNAL(finished(QNetworkReply*)), SLOT(replyFinished(QNetworkReply*)));
    m_authenticationState = NoAuthentication;
}

QtGoogleClientLogin::~QtGoogleClientLogin()
{
    delete m_networkAccessManager;
}

void QtGoogleClientLogin::setSettingsKey(const QString &key)
{
    m_settingsKey = key;
    QSettings settings;
    settings.beginGroup(m_settingsKey);
    m_login = settings.value("user1").toString();
    m_password = settings.value("user2").toString();
    settings.endGroup();
}

bool QtGoogleClientLogin::isConfiguredWithSettings() const
{
    return !m_login.isEmpty() && !m_password.isEmpty();
}

void QtGoogleClientLogin::setLogin(const QString &login)
{
    m_login = login;
}

void QtGoogleClientLogin::setPassword(const QString &password)
{
    m_password = password;
}

void QtGoogleClientLogin::setServiceName(const QString &serviceName)
{
    m_serviceName = serviceName;
}

void QtGoogleClientLogin::setSourceName(const QString &sourceName)
{
    m_sourceName = sourceName;
}

void QtGoogleClientLogin::setCaptchaAnswer(const QString &captchaAnswer)
{
    m_captchaAnswer = captchaAnswer;
}

AuthenticationState QtGoogleClientLogin::authenticate()
{
    sendAuthenticationRequest();
    QEventLoop loop;
    QObject::connect(this, SIGNAL(authenticationResponse(AuthenticationState)), &loop, SLOT(quit()));
    loop.exec();
    return m_authenticationState;
}

void QtGoogleClientLogin::sendAuthenticationRequest()
{
    m_authenticationState = PendingAuthentication;

    //qDebug() << "logging in" << m_login << "pass" << m_password.count() << "chars";
    //qDebug() << "ssl socket support" << QSslSocket::supportsSsl();

    QNetworkRequest request(QUrl("https://www.google.com/accounts/ClientLogin"));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");

    QByteArray data;
    QUrl params;
    params.addQueryItem("accountType", "HOSTED_OR_GOOGLE");
    params.addQueryItem("Email", m_login);
    params.addQueryItem("Passwd", m_password);
    params.addQueryItem("service", m_serviceName);
    params.addQueryItem("source", m_sourceName);

    data.append(params.toString());
    data.remove(0,1);

    m_reply = m_networkAccessManager->post(request, data);
    connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(slotError(QNetworkReply::NetworkError)));
    connect(m_reply, SIGNAL(sslErrors(QList<QSslError>)),
            this, SLOT(slotSslErrors(QList<QSslError>)));
}

QByteArray QtGoogleClientLogin::authenticationToken()
{
    return m_auth;
}

void QtGoogleClientLogin::replyFinished(QNetworkReply *reply)
{
    reply->deleteLater();
    if (m_authenticationState != PendingAuthentication)
        return; // there was an error

    m_replyContents = m_reply->readAll();
    QList<QByteArray> replyParts = m_replyContents.split('\n');
    foreach (const QByteArray &replyPart, replyParts) {
        if (replyPart.startsWith("Auth")) {
            m_auth = replyPart;
        }
    }
    if (m_auth.isEmpty()) {
       m_authenticationState = NetworkError;
    } else {
        m_authenticationState = SuccessfullAuthentication;
        // Save login details on successful auth
        if (!m_settingsKey.isEmpty()) {
            QSettings settings;
            settings.beginGroup(m_settingsKey);
            settings.setValue("user1", QVariant(m_login));
            settings.setValue("user2", QVariant(m_password));
            settings.endGroup();
        }
    }
    emit authenticationResponse(m_authenticationState);
}

void QtGoogleClientLogin::slotError(QNetworkReply::NetworkError error)
{
    m_replyContents = m_reply->readAll();
    if (error == 202) {
        QList<QByteArray> replyParts = m_replyContents.split('\n');
        if (replyParts.contains(QByteArray("Error=CaptchaRequired"))) {
            m_authenticationState = CaptchaRequired;
        } else {
            m_authenticationState = FailedAuthentication;
        }
    } else {
        m_authenticationState = NetworkError;
    }

    emit authenticationResponse(m_authenticationState);
}

void QtGoogleClientLogin::slotSslErrors(QList<QSslError> sslErrors)
{
    m_reply->ignoreSslErrors();
}



