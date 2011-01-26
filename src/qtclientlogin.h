#ifndef QTCLIENTLOGIN_H
#define QTCLIENTLOGIN_H

#include <QObject>
#include <QtGui>
#include <QtNetwork>
#include <QSslError>

/*
    API Docs at http://code.google.com/apis/accounts/docs/AuthForInstalledApps.html

    Usage:

    setLogin("mymail@google.com")
    setPassword("********")
    setServiceName("foo")  Pick one from http://code.google.com/apis/gdata/faq.html#clientlogin
    setSourceName("MyCompany-MyApp-MyVersionNumber")

    sendAuthenticationRequest()
    Handle Signal: authenticationResponse();

    ... (TODO)
*/
class QtGoogleClientLogin : public QObject
{
    Q_OBJECT
public:
    explicit QtGoogleClientLogin(QObject *parent = 0);
    ~QtGoogleClientLogin();

    void setLogin(const QString &login);
    void setPassword(const QString &password);
    void setServiceName(const QString &serviceName);
    void setSourceName(const QString &serviceName);

    void setCaptchaAnswer(const QString &captchaAnswer);
    void sendAuthenticationRequest();

    enum AuthenticationReponse { Success, Failure, InvalidRequest, Captcha};
    QImage captcha() const;
    QByteArray authenticationToken();
    QString statusCode();
signals:
    void authenticationResponse();

public slots:

private slots:
    void replyFinished(QNetworkReply *);
    void slotReadyRead();
    void slotError(QNetworkReply::NetworkError);
    void slotSslErrors(QList<QSslError>);
private:
    QNetworkAccessManager *m_networkAccessManager;
    QString m_login;
    QString m_password;
    QString m_serviceName;
    QString m_sourceName;
    QImage m_captcha;
    QString m_captchaAnswer;
    QString m_status;
    QNetworkReply *m_reply;
};

#endif // QTCLIENTLOGIN_H
