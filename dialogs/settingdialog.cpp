
#include "settingdialog.h"
#include <QVBoxLayout>
#include <QTabWidget>


SettingDialog::SettingDialog(QWidget *parent) :
    FBaseDialog(parent)
{
    initUI();
}

void SettingDialog::initUI()
{
    normalSize = QSize(330, 310);
    getTitleBar()->getTitleLabel()->setText(tr("Settings"));
    QVBoxLayout* mainLayout = (QVBoxLayout*)layout();

    SerialPortSettingsDialog *_serialport = new SerialPortSettingsDialog;


    QWidget* tab2 = new QWidget;
    QWidget* tab3 = new QWidget;

    QTabWidget* tabwidget = new QTabWidget;


    tabwidget->addTab(_serialport, tr("SerialPort"));
    tabwidget->addTab(tab2, tr("CANPort"));
    tabwidget->addTab(tab3, tr("TcpPort"));
    mainLayout->addSpacing(5);
    mainLayout->addWidget(tabwidget);

    connect(_serialport, SIGNAL(emitApply()), this, SLOT(close()));
}
