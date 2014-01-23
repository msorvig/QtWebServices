#include <QtCore/QObject>
#include <qtgoogleloginwidget.h>
#include <qtgoogleloginwindowcontroller.h>

class AppLogic : public QObject
{
Q_OBJECT
public:
    AppLogic();
    void start();

private Q_SLOTS:
    void displayLoginPage(const QUrl &rul);
    void loginCompleted(const QString &webPageTitle);
    void accessTokenReady(const QString &accessToken);
    void refreshTokenReady(const QString &refreshToken);
    void error(const QString &werror);
private:
    QtGoogleOAuthLogin *login;
    QWidget *window;
    QLabel *label;

//#define LOGIN_WIDGET
#ifdef LOGIN_WIDGET
    QtGoogleLoginWidget *loginWidget;
#endif
    QtGoogleLoginWindowController *loginWindowController;
};
