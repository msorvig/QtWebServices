#include "applogic.h"
#include "qtpicasaweb.h"
#include "qtpicasaloader.h"
#include <QtWidgets>

AppLogic::AppLogic()
{
    login = new QtGoogleOAuthLogin(this);
    login->setClientId(QStringLiteral("***********.apps.googleusercontent.com"));
    login->setClientSecret(QStringLiteral("********************"));
    login->setScope(QStringLiteral("http://picasaweb.google.com/data/"));

    QSettings settings;
    login->setRefreshToken(settings.value("refreshToken").toString());
    // Comment out refresh token to start login from scratch

    connect(login, SIGNAL(displayLoginPage(QUrl)), SLOT(displayLoginPage(QUrl)));
    connect(login, SIGNAL(accessTokenReady(QString)), SLOT(accessTokenReady(QString)));
    connect(login, SIGNAL(refreshTokenReady(QString)), SLOT(refreshTokenReady(QString)));
    connect(login, SIGNAL(error(QString)), SLOT(error(QString)));

    window = new QWidget();
}

void AppLogic::start()
{
    window->resize(600, 600);

    label = new QLabel;
    label->setText("Hello World");

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(label);
    window->setLayout(layout);
    window->show();

    login->initiateAccess();
}

void AppLogic::displayLoginPage(const QUrl &rul)
{
#ifdef LOGIN_WIDGET
    loginWidget = new QtGoogleLoginWidget();
    loginWidget->move(50, 50);
    loginWidget->setWindowTitle("QtGoogleLoginWidget");
    connect(loginWidget, SIGNAL(pageTitleChanged(QString)), SLOT(handleLoginPageLoad(QString)));
    loginWidget->displayLoginPage(login->authorizationUrl());
#else
    loginWindowController = new QtGoogleLoginWindowController(window->windowHandle());
    connect(loginWindowController, SIGNAL(loginCompleted(QString)), SLOT(loginCompleted(QString)));
    loginWindowController->displayLoginPage(login->authorizationUrl());
#endif
}

void AppLogic::loginCompleted(const QString &result)
{
//    qDebug() << "loginCompleted" << result;
#ifdef LOGIN_WIDGET
    loginWidget->hide();
    delete loginWidget;
#else
    loginWindowController->hide();
    delete loginWindowController;
#endif
    login->initiateAccessFromLoginWebPageTitle(webPageTitle);
}

void AppLogic::accessTokenReady(const QString &accessToken)
{
    qDebug() << "accessTokenReady" << accessToken;

    label->setText(QStringLiteral("AccessToken: ") +  accessToken);
    QtPicasaLoader picasaLoader("temptemptemp");
    picasaLoader.setAccessToken(accessToken);
    picasaLoader.downloadFeedXml();
    picasaLoader.parseFeed();
}

void AppLogic::refreshTokenReady(const QString &refreshToken)
{
    qDebug() << "refreshTokenReady" << refreshToken;
    QSettings settings;
    settings.setValue("refreshToken", refreshToken);
}

void AppLogic::error(const QString &error)
{
    qDebug() << "Login Error" << error;
}


