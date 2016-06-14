#include "serialportui.h"
#include "ui_serialportui.h"

SerialPortUi::SerialPortUi(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SerialPortUi)
{
    ui->setupUi(this);
    initData();
    initUI();
    initThread();
    initConnect();
}
void SerialPortUi::initData()
{
    mySerialPortThread = SerialPortThread::getInstance();

}

void SerialPortUi::initUI()
{

}

void SerialPortUi::initThread()
{

}

void SerialPortUi::initConnect()
{
    connect(mySerialPortThread, SIGNAL(message(QString)),this, SLOT(messageShow(QString)));
}
void SerialPortUi::messageShow(const QString &s)
{

    ui->textBrowser->append(s);
}
SerialPortUi::~SerialPortUi()
{
    delete ui;
}

void SerialPortUi::radioButton_clicked()
{
    QRadioButton *rtn = dynamic_cast<QRadioButton*>(sender());
    int key = rtn->whatsThis().toInt();
    switch (key) {
    case 1:

        break;
    case 2:

        break;
    case 3:

        break;
    default:
        break;
    }
}
