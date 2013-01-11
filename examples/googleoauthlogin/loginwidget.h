#include <QtWidgets/QtWidgets>

#include <qtmaccocoaviewcontainer.h>
#include <qtgoogleoauthlogin.h>
#include "qtwebview.h"

class LoginWidget : public QWidget
{
Q_OBJECT
public:
    LoginWidget(QWidget *parentWidget = 0);
Q_SIGNALS:
    void pageTitleChanged(const QString &pageTitle);
public Q_SLOTS:
    void displayLoginPage(const QUrl &loginUrl);
private Q_SLOTS:
    void pageLoadFinished(bool ok);
private:
    QtWebView *webView;
    QtMacCocoaViewContainer *webViewContainer;
};
