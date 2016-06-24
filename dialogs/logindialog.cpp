
#include "logindialog.h"

#include <QVBoxLayout>

#include "mainwindow/mainwindow.h"


#include "userui/loginframe.h"

LoginDialog::LoginDialog(QWidget *parent):
    FBaseDialog(parent)
{
    initUI();

}

LoginDialog::~LoginDialog()
{

}

void LoginDialog::initUI()
{
    normalSize = QSize(500, 300);
    getTitleBar()->getTitleLabel()->setText(tr("Login"));
    QVBoxLayout* mainLayout = (QVBoxLayout*)layout();



    LoginFrame *_login = new LoginFrame;
    mainLayout->addWidget(_login);

    MainWindow *main = MainWindow::getInstance();
    main->setAttribute(Qt::WA_DeleteOnClose);

    connect(_login, SIGNAL(login()), this, SLOT(hide()));
    connect(_login, SIGNAL(login()), main, SLOT(show()));
}
