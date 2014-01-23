#include "qtgoogleloginwidget.h"

QtGoogleLoginWidget::QtGoogleLoginWidget(QWidget *parentWidget)
    :QWidget(parentWidget)
{
    webView = new QtWebView(this);
    webViewContainer = new QMacCocoaViewContainer((NSView *)(webView->nativeView()), this);
    webViewContainer->setGeometry(0, 0, 500, 500);

    connect(webView, SIGNAL(loadFinished(bool)), SLOT(pageLoadFinished(bool)));
}

void QtGoogleLoginWidget::displayLoginPage(const QUrl &loginUrl)
{
    webView->load(loginUrl);
    show();
}

void QtGoogleLoginWidget::pageLoadFinished(bool ok)
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
