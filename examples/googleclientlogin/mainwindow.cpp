#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qtgoogleclientlogin.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_clientLogin = new QtGoogleClientLogin();
    connect(m_clientLogin, SIGNAL(authenticationResponse()), SLOT(onAuthenticationResponse()));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_clientLogin;
}

void MainWindow::on_pushButtonGo_clicked()
{
    ui->pushButtonGo->setDisabled(true);
    m_clientLogin->setLogin(ui->lineEditLogin->text());
    m_clientLogin->setPassword(ui->lineEditPassword->text());
    m_clientLogin->setServiceName("lh2"); // Picasa API 2.0
    m_clientLogin->setSourceName("ExampleCom-QtGoogleClient-0.1"); // company-app-version
    m_clientLogin->sendAuthenticationRequest();
}

void MainWindow::onAuthenticationResponse()
{
    qDebug() << "response";
    ui->textBrowserResponse->setText("response " + m_clientLogin->statusCode());
}
