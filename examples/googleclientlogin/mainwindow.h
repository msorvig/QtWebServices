#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <qtgoogleclientlogin.h>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QtGoogleClientLogin *m_clientLogin;

private slots:
    void on_pushButtonGo_clicked();
    void onAuthenticationResponse(AuthenticationState response);
};

#endif // MAINWINDOW_H
