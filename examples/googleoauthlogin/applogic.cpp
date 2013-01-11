#include "applogic.h"

#include <QtWidgets>

AppLogic::AppLogic()
{
    login = new QtGoogleOAuthLogin(this);
    login->setClientId(QStringLiteral("***********.apps.googleusercontent.com"));
    login->setClientSecret(QStringLiteral("********************"));
    login->setScope(QStringLiteral("http://picasaweb.google.com/data/"));

    QSettings settings;
    login->setRefreshToken(settings.value("refreshToken").toString());
    //login->setRefreshToken("foo");

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

void AppLogic::displayLoginPage(const QUrl &rul)
{
    loginWidget = new LoginWidget();
    loginWidget->move(50, 50);
    connect(loginWidget, SIGNAL(pageTitleChanged(QString)), SLOT(handleLoginPageLoad(QString)));
    loginWidget->displayLoginPage(login->authorizationUrl());
}

void AppLogic::handleLoginPageLoad(const QString &webPageTitle)
{
    loginWidget->hide();
    delete loginWidget;
    login->initiateAccessFromLoginWebPageTitle(webPageTitle);
}

void AppLogic::accessTokenReady(const QString &accessToken)
{
    qDebug() << "accessTokenReady" << accessToken;

    label->setText(QStringLiteral("AccessToken: ") +  accessToken);
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


