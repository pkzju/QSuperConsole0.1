#include "loginframe.h"
#include "ui_loginframe.h"


LoginFrame::LoginFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::LoginFrame)
{
    ui->setupUi(this);
}

LoginFrame::~LoginFrame()
{
    delete ui;
}

void LoginFrame::on_pushButton_clicked()
{
    if(ui->accountText->text().trimmed() == "admin")
    {
        if(ui->passwordText->text().trimmed() == "admin")
        {
            emit login();
        }
        else
        {
            //QMessageBox::warning(this,"warning",QStringLiteral("密码错误"));
        }
    }
}


