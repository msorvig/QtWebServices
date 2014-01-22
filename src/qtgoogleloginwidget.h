#include <QtWidgets/QtWidgets>
#include <QtWidgets/QMacCocoaViewContainer>

#include <qtgoogleoauthlogin.h>
#include "qtwebview.h"

class QtGoogleLoginWidget : public QWidget
{
Q_OBJECT
public:
    QtGoogleLoginWidget(QWidget *parentWidget = 0);
Q_SIGNALS:
    void pageTitleChanged(const QString &pageTitle);
public Q_SLOTS:
    void displayLoginPage(const QUrl &loginUrl);
private Q_SLOTS:
    void pageLoadFinished(bool ok);
private:
    QtWebView *webView;
    QMacCocoaViewContainer *webViewContainer;
};
