
#include "settingdialog.h"
#include <QVBoxLayout>
#include <QTabWidget>

#include"userui/serialportsettingsdialog.h"
#include"userui/tcpportsettings.h"

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

    SerialPortSettingsDialog *_serialport = new SerialPortSettingsDialog(this);
    TcpPortSettings *_tcpPortSettings = new TcpPortSettings(this);

    QWidget* tab2 = new QWidget;

    QTabWidget* tabwidget = new QTabWidget;


    tabwidget->addTab(_serialport, tr("SerialPort"));
    tabwidget->addTab(tab2, tr("CANPort"));
    tabwidget->addTab(_tcpPortSettings, tr("TcpPort"));
    mainLayout->addSpacing(5);
    mainLayout->addWidget(tabwidget);

    connect(_serialport, SIGNAL(emitApply()), this, SLOT(close()));
    connect(_tcpPortSettings, SIGNAL(emitApply()), this, SLOT(close()));
}
