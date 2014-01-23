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

    //QSplitter *splitter = new QSplitter;
    //QListWidget *listView = new QListWidget;
    //listView->addItems(QStringList() << "en" << "to" << "tre");

    label = new QLabel;
    label->setText("");

    //splitter->addWidget(listView);
    //splitter->addWidget(label);
    //splitter->setSizes(QList<int>() << 20 << 50);
    QHBoxLayout *layout = new QHBoxLayout;

    layout->addWidget(label);
    window->setLayout(layout);
    window->show();

    login->initiateAccess();
}

//#define LOGIN_WIDGET
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
    connect(loginWindowController, SIGNAL(pageTitleChanged(QString)), SLOT(handleLoginPageLoad(QString)));
    loginWindowController->displayLoginPage(login->authorizationUrl());
#endif
}

void AppLogic::handleLoginPageLoad(const QString &webPageTitle)
{
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
/*
    QtPicasaWeb picasaweb;
    picasaweb.setAuthenticationToken(accessToken);
    QByteArray feed = picasaweb.requestFeed();
//    qDebug() << "feed" << feed.count();
    picasaweb.parseFeedXml(feed);
*/
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
    qDebug() << "error error error" << error;
}


