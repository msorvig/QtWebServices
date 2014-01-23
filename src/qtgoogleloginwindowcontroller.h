#include "qtwebview.h"
#include <QtGui/QWindow>

class QtGoogleLoginWindowController : public QObject
{
Q_OBJECT
public:
    explicit QtGoogleLoginWindowController(QWindow *parent);
    ~QtGoogleLoginWindowController();
    void hide();
Q_SIGNALS:
    void loginCompleted(const QString &result);
public Q_SLOTS:
    void displayLoginPage(const QUrl &loginUrl);
private Q_SLOTS:
    void pageLoadFinished(bool ok);
private:
    QtWebView *webView;
    QWindow *window;
};
