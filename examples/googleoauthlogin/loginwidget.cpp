#include "loginwidget.h"

LoginWidget::LoginWidget(QWidget *parentWidget)
    :QWidget(parentWidget)
{
    webView = new QtWebView(this);
    webViewContainer = new QtMacCocoaViewContainer(webView->nativeView(), this);
    webViewContainer->setGeometry(0, 0, 500, 500);

    connect(webView, SIGNAL(loadFinished(bool)), SLOT(pageLoadFinished(bool)));
}

void LoginWidget::displayLoginPage(const QUrl &loginUrl)
{
    webView->load(loginUrl);
    show();
}

void LoginWidget::pageLoadFinished(bool ok)
{
    QString webPageTitle = webView->title();
    if (webPageTitle.startsWith(QStringLiteral("Request for Permission")))
        return; // Initial login page

    // Result page
    emit pageTitleChanged(webPageTitle);
}