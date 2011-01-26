#include "qtgoogleclientlogin.h"
#include <QtNetwork>
#include <QSslError>
#include <QSslSocket>

QtGoogleClientLogin::QtGoogleClientLogin(QObject *parent)
    : QObject(parent)
{
    m_networkAccessManager = new QNetworkAccessManager(this);
    connect(m_networkAccessManager, SIGNAL(finished(QNetworkReply*)), SLOT(replyFinished(QNetworkReply*)));
}

QtGoogleClientLogin::~QtGoogleClientLogin()
{
    delete m_networkAccessManager;
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

void QtGoogleClientLogin::sendAuthenticationRequest()
{
    qDebug() << "logging in" << m_login << "pass" << m_password.count() << "chars";
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
    connect(m_reply, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
    connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(slotError(QNetworkReply::NetworkError)));

   connect(m_reply, SIGNAL(sslErrors(QList<QSslError>)),
           this, SLOT(slotSslErrors(QList<QSslError>)));

    qDebug() << m_reply->isFinished() << m_reply->isRunning();
}

QString QtGoogleClientLogin::statusCode()
{
    m_status = "unicorns and rainbows";
    return m_status;
}

void QtGoogleClientLogin::replyFinished(QNetworkReply *reply)
{
    qDebug() << "reply" << m_reply->readAll();
}

void QtGoogleClientLogin::slotReadyRead()
{
    qDebug() << "ready read";
}

void QtGoogleClientLogin::slotError(QNetworkReply::NetworkError error)
{
    qDebug() << "network error" << error;
    qDebug() << m_reply->readAll();
}


void QtGoogleClientLogin::slotSslErrors(QList<QSslError> sslErrors)
{
    qDebug() << "ssl error" << sslErrors;
    m_reply->ignoreSslErrors();

}



