/****************************************************************************
**
** Copyright (C) 2016 pkzju
**
** Version	: 0.0.0.1
** Author	: pkzju
** 
****************************************************************************/

#include "mainwindow.h"
#include "centerwindow.h"
#include "QFramer/futil.h"
#include "settingmenu.h"
#include "thememenu.h"
#include "functionpages/rightfloatwindow.h"
#include "lamp/qcw_indicatorlamp.h"

#include "dialogs/siglegroupdialog.h"
#include"userui/serialportsettingsdialog.h"

#include <QtCore>
#include <QModbusRtuSerialMaster>


MainWindow* MainWindow::instance = Q_NULLPTR;

MainWindow::MainWindow(QWidget *parent) :
    FMainWindow(parent),
    centerWindow(new CenterWindow), settingMenu (new SettingMenu),
    themeMenu(new ThemeMenu), rightfloatWindow(new RightFloatWindow)
{
    qDebug("mainwindow init");
    initData();
    initUI();
    initThread();
    initConnect();

}

MainWindow::~MainWindow()
{
    //! Delete static sigleGroupDialog (include static fanmotorui)
//    sigleGroupDialog::deleteInstance();

    if(modbusRtuDevice && modbusRtuDevice->state() == QModbusDevice::ConnectedState){
        modbusRtuDevice->disconnectDevice();
        modbusRtuDevice->deleteLater();
    }
    instance = Q_NULLPTR;
    qDebug("mainwindow exit");
}

void MainWindow::initData()
{
}

void MainWindow::initUI()
{
    //! set centerWindow
    setCentralWidget(centerWindow);
    centerWindow->getNavgationBar()->setCurrentIndex(0);

    //! set setting Menu
    getTitleBar()->getSettingButton()->setMenu(settingMenu);
    getQSystemTrayIcon()->setContextMenu(settingMenu);
    getFlyWidget()->setMenu(settingMenu);

    //! set theme Menu
    getTitleBar()->getSkinButton()->setMenu(themeMenu);

    QGridLayout *gdLayout = new QGridLayout();
    rightfloatWindow->setLayout(gdLayout);

    LampOfStatusBar =new QcwIndicatorLamp;
    LampOfStatusBar->setFixedSize(20,20);

    OpenOfStatusBar = new QToolButton;
    OpenOfStatusBar->setIcon(QIcon(":/images/skin/images/connect.png"));
    OpenOfStatusBar->setAutoRaise(true);
    OpenOfStatusBar->setIconSize(QSize(25,40));
    CloseOfStatusBar = new QToolButton;
    CloseOfStatusBar->setIcon(QIcon(":/images/skin/images/disconnect.png"));
    CloseOfStatusBar->setIconSize(QSize(25,40));
    CloseOfStatusBar->setAutoRaise(true);

    getStatusBar()->addPermanentWidget(OpenOfStatusBar);//at the right
    getStatusBar()->addPermanentWidget(CloseOfStatusBar);
    getStatusBar()->addPermanentWidget(LampOfStatusBar);

    connect(OpenOfStatusBar, SIGNAL(clicked()), this, SLOT(onConnectButtonClicked()));
    connect(CloseOfStatusBar, SIGNAL(clicked()), this, SLOT(onDisconnectButtonClicked()));

}

void MainWindow::onConnectButtonClicked()
{
    //! Modbus RTU connect

    SerialPortSettings::Settings mSerialPortSettings;
    //![1] Read serialport settings in "/SuperConsole.ini" (update in "SerialPortSettingsDialog" class)
    QSettings settings(QDir::currentPath() + "/SuperConsole.ini", QSettings::IniFormat);
    settings.beginGroup("SerialPortSettingsDialog");
    mSerialPortSettings.name = settings.value("SerialPortname", "").toString();
    mSerialPortSettings.baudRate = static_cast<QSerialPort::BaudRate>(settings.value("SerialPortBaudRate", "9600").toInt());
    mSerialPortSettings.dataBits = static_cast<QSerialPort::DataBits>(settings.value("SerialPortDataBits", "8").toInt());
    mSerialPortSettings.parity = static_cast<QSerialPort::Parity>(settings.value("SerialPortParity", "0").toInt());
    mSerialPortSettings.stopBits = static_cast<QSerialPort::StopBits>(settings.value("SerialPortStopBits", "1").toInt());
    mSerialPortSettings.flowControl = static_cast<QSerialPort::FlowControl>(settings.value("SerialPortFlowControl", "0").toInt());
    settings.endGroup();

    //![2] Make sure modbus device was not created yet
    if (modbusRtuDevice) {
        modbusRtuDevice->disconnectDevice();
        modbusRtuDevice->deleteLater();
        modbusRtuDevice = Q_NULLPTR;
    }

    //![3] Create modbus RTU client device and monitor error,state
    modbusRtuDevice = new QModbusRtuSerialMaster(this);
    connect(modbusRtuDevice, &QModbusClient::errorOccurred, [this](QModbusDevice::Error) {
        statusBar()->showMessage(modbusRtuDevice->errorString(), 5000);
    });
    if (!modbusRtuDevice) {
        statusBar()->showMessage(tr("Could not create Modbus client."), 5000);
        return;
    } else {
        connect(modbusRtuDevice, &QModbusClient::stateChanged,
                this, &MainWindow::onStateChanged);//!< Monitor modbus device state
    }

    statusBar()->clearMessage();//!< Clear status bar

    //![4] Set serialport parameter and connect device
    if (modbusRtuDevice->state() != QModbusDevice::ConnectedState) {

        modbusRtuDevice->setConnectionParameter(QModbusDevice::SerialPortNameParameter,
                                                mSerialPortSettings.name);
        modbusRtuDevice->setConnectionParameter(QModbusDevice::SerialBaudRateParameter,
                                                mSerialPortSettings.baudRate);
        modbusRtuDevice->setConnectionParameter(QModbusDevice::SerialDataBitsParameter,
                                                mSerialPortSettings.dataBits);
        modbusRtuDevice->setConnectionParameter(QModbusDevice::SerialParityParameter,
                                                mSerialPortSettings.parity);
        modbusRtuDevice->setConnectionParameter(QModbusDevice::SerialStopBitsParameter,
                                                mSerialPortSettings.stopBits);

        modbusRtuDevice->setTimeout(100);//!< Set response timeout 100ms
        modbusRtuDevice->setNumberOfRetries(2);//!< Set retry number 2

        //! Now try connect modbus device
        if (!modbusRtuDevice->connectDevice()) {//!< Connect failed
            statusBar()->showMessage(tr("Modbus connect failed: ") + modbusRtuDevice->errorString(), 5000);
        } else {//!< Connect  successfully
        }
    }


}

/*!
 * \brief Disconnect current connected device
 */
void MainWindow::onDisconnectButtonClicked()//! close
{
    //! Modbus RTU disconnect

    if(!modbusRtuDevice)
        return;
    if(modbusRtuDevice->state() == QModbusDevice::ConnectedState){
        modbusRtuDevice->disconnectDevice();
    }
}

/*!
 * \brief Current device state changed  slot fuction
 * \param state
 */
void MainWindow::onStateChanged(int state)
{
    bool connected = (state != 0);

    //! Modbus RTU state change

    connected = (state == QModbusDevice::ConnectedState);
    if(connected){
        statusBar()->showMessage(tr("Modbus RTU connected !"), 5000);
    }
    else{
        statusBar()->showMessage(tr("Modbus RTU disonnected !"), 5000);
    }

    OpenOfStatusBar->setEnabled(!connected);
    CloseOfStatusBar->setEnabled(connected);
    setStatusBarLamp(connected);

}


void MainWindow::setStatusBarLamp(bool isOpen)
{
    if(isOpen)
        LampOfStatusBar->setLampState(QcwIndicatorLamp::lamp_green);
    else
        LampOfStatusBar->setLampState(QcwIndicatorLamp::lamp_grey);

}

QToolButton *MainWindow::getOpenButton()
{
    return OpenOfStatusBar;
}

QToolButton *MainWindow::getCloseButton()
{
    return CloseOfStatusBar;
}

QModbusClient *MainWindow::getRtuDevice()
{
    return modbusRtuDevice;
}

QModbusReply *MainWindow::sendRtuReadRequest(const QModbusDataUnit &read, int serverAddress)
{
    if(!modbusRtuDevice || modbusRtuDevice->state() != QModbusDevice::ConnectedState)
        return false;
    if (auto *reply = modbusRtuDevice->sendReadRequest(read, serverAddress)) {
        return reply;
    } else {
        statusBar()->showMessage(tr("Read error: ") + modbusRtuDevice->errorString());
        return Q_NULLPTR;
    }

}
bool MainWindow::sendRtuWriteRequest(QModbusDataUnit unit, quint16 serverAddress)
{
    if(!modbusRtuDevice || modbusRtuDevice->state() != QModbusDevice::ConnectedState)
        return false;

    //! Send write request to server (adress: )
    if (auto *reply = modbusRtuDevice->sendWriteRequest(unit, serverAddress)) {
        if (!reply->isFinished()) {

            connect(reply, &QModbusReply::finished, this, [this, reply]() {

                if(reply->serverAddress() != 0){//! Not broadcast
                    if (reply->error() == QModbusDevice::ProtocolError) {
                         statusBar()->showMessage(tr("Write response error: %1 (Mobus exception: 0x%2)")
                                                .arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 16)
                                                );
                    } else if (reply->error() != QModbusDevice::NoError) {
                         statusBar()->showMessage(tr("Write response error: %1 (code: 0x%2)").
                                                arg(reply->errorString()).arg(reply->error(), -1, 16));
                    }
                }
                reply->deleteLater();
            });
        } else {//!< Broadcast replies return immediately
            reply->deleteLater();
        }
    } else {
         statusBar()->showMessage(tr("Write error: ") + modbusRtuDevice->errorString());
        return false;
    }

    return true;
}

void MainWindow::initThread()
{

}

void MainWindow::initConnect()
{
    connect(this, SIGNAL(Hidden()), rightfloatWindow, SLOT(hide()));

}

MainWindow* MainWindow::getInstance()
{
    if(!instance)
    {
        instance = new MainWindow();
    }
    return instance;
}

SettingMenu* MainWindow::getSettingMenu()
{
    return settingMenu;
}

RightFloatWindow* MainWindow::getRightFloatWindow()
{
    return rightfloatWindow;
}


void MainWindow::resizeEvent(QResizeEvent *event)
{
    rightfloatWindow->resize(rightfloatWindow->width(), event->size().height());
    rightfloatWindow->hide();
    QMainWindow::resizeEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    rightfloatWindow->move(x() + width() + 2, y());
    FMainWindow::mouseMoveEvent(event);
}

void MainWindow::showEvent(QHideEvent *event)
{
    rightfloatWindow->setVisible(true);
    event->accept();
}

void MainWindow::hideEvent(QHideEvent *event)
{
    rightfloatWindow->setVisible(false);
    event->accept();
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->y() > getTitleBar()->height())
    {
        rightfloatWindow->setVisible(! rightfloatWindow->isVisible());
    }
    FMainWindow::mouseDoubleClickEvent(event);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Left)
    {
        int index = centerWindow->getNavgationBar()->currentIndex();
        int count = centerWindow->getNavgationBar()->count();
        if(index == 0){
            centerWindow->getNavgationBar()->setCurrentIndex(count - 1);
        }else if(index <= (count - 1) && index > 0){
            centerWindow->getNavgationBar()->setCurrentIndex(index - 1);
        }
    }
    else if(event->key() == Qt::Key_Right)
    {
        int index = centerWindow->getNavgationBar()->currentIndex();
        int count = centerWindow->getNavgationBar()->count();
        if(index == (count - 1)){
            centerWindow->getNavgationBar()->setCurrentIndex(0);
        }else if(index >= 0 && index < (count - 1)){
            centerWindow->getNavgationBar()->setCurrentIndex(index + 1);
        }
    }
    else if(event->key() == Qt::Key_F1)
    {
        delete centerWindow->getNavgationBar()->layout();
        centerWindow->setAlignment(centerWindow->TopCenter);
    }
    else if(event->key() == Qt::Key_Space)
    {
        delete centerWindow->getNavgationBar()->layout();
        int i = 1 + (int)12 * rand() / (RAND_MAX + 1);
        switch (i) {
        case 1:
            centerWindow->setAlignment(centerWindow->TopLeft);
            break;
        case 2:
            centerWindow->setAlignment(centerWindow->TopCenter);
            break;
        case 3:
            centerWindow->setAlignment(centerWindow->TopRight);
            break;
        case 4:
            centerWindow->setAlignment(centerWindow->RightTop);
            break;
        case 5:
            centerWindow->setAlignment(centerWindow->RightCenter);
            break;
        case 6:
            centerWindow->setAlignment(centerWindow->RightBottom);
            break;
        case 7:
            centerWindow->setAlignment(centerWindow->BottomRight);
            break;
        case 8:
            centerWindow->setAlignment(centerWindow->BottomCenter);
            break;
        case 9:
            centerWindow->setAlignment(centerWindow->BottomLeft);
            break;
        case 10:
            centerWindow->setAlignment(centerWindow->LeftBottom);
            break;
        case 11:
            centerWindow->setAlignment(centerWindow->LeftCenter);
            break;
        case 12:
            centerWindow->setAlignment(centerWindow->LeftTop);
            break;
        default:
            break;
        }
    }

    FMainWindow::keyPressEvent(event);
}
