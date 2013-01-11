#include <QtCore/QObject>
#include "loginwidget.h"

class AppLogic : public QObject
{
Q_OBJECT
public:
    AppLogic();
    void start();

    void displayLoginPage();
private Q_SLOTS:
    void handleLoginPageLoad(const QString &webPageTitle);
    void accessTokenReady(const QString &accessToken);
    void refreshTokenReady(const QString &refreshToken);
    void error(const QString &werror);
private:
    QtGoogleOAuthLogin *login;
    QWidget *window;
    QLabel *label;
    LoginWidget *loginWidget;
};
