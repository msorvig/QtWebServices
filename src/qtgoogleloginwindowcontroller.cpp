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
    QString pageTitle = webView->title();

   // As the user progresses through the login sequence there
   // will be multiple page load events. The web page title
   // containes the (possibly internationalized) status.
   //
   // The final page title will contain the quthorization code:
   //   Success state=authenticae&code=*****
   // Or deny access:
   //   Denied error=access_denied&state=authenticae"
   if (!pageTitle.contains("Success") && !pageTitle.contains("Denied"))
        return; // login still in progress

    emit loginCompleted(pageTitle);
}
