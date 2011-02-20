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
    Handle Signal: authenticationResponse(), deal with authenticationState as specified below.
*/
enum AuthenticationState {
    NoAuthentication,
    PendingAuthentication,      // lookup in progress, wait.
    SuccessfullAuthentication,  // login and password was accepted, read authenticationToken for key
    FailedAuthentication,       // login or password mismatch.
    CaptchaRequired,            // [display captcha, set answer, call sendAuthenticationRequest again. NOT IMPLEMENTED]
                                // alternatively, have the user solve the captcha at
                                // https://www.google.com/accounts/DisplayUnlockCaptcha
    NetworkError                // network on fire.
};
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

    void sendAuthenticationRequest();

    // Not implemented
    //QImage captcha() const;
    void setCaptchaAnswer(const QString &captchaAnswer);

    QByteArray authenticationToken();
    AuthenticationState authenticationState();
signals:
    void authenticationResponse(AuthenticationState response);

private slots:
    void replyFinished(QNetworkReply *);
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
    QByteArray m_replyContents;
    QByteArray m_auth;
    QNetworkReply *m_reply;
    AuthenticationState m_authenticationState;
};

#endif // QTCLIENTLOGIN_H
