#include "qtgoogleloginwindowcontroller.h"


QtGoogleLoginWindowController::QtGoogleLoginWindowController(QWindow *parent)
{
    webView = new QtWebView(this);
    window = QWindow::fromWinId((WId)(webView->nativeView()));
    window->setGeometry(50, 50, 500, 500);
    window->setParent(parent);

    connect(webView, SIGNAL(loadFinished(bool)), SLOT(pageLoadFinished(bool)));
}

QtGoogleLoginWindowController::~QtGoogleLoginWindowController()
{
    delete webView;
    delete window;
}

void QtGoogleLoginWindowController::hide()
{
    window->hide();
}

void QtGoogleLoginWindowController::displayLoginPage(const QUrl &loginUrl)
{
    webView->load(loginUrl);
    window->show();
}

void QtGoogleLoginWindowController::pageLoadFinished(bool ok)
{
    QString webPageTitle = webView->title();
    if (webPageTitle.startsWith(QStringLiteral("Request for Permission")))
        return; // Initial login page

    // Result page
    emit pageTitleChanged(webPageTitle);
}
