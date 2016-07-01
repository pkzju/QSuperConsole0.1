#include "fanmotorui.h"
#include "ui_fanmotorui.h"

#include <QModbusTcpClient>
#include <QModbusRtuSerialMaster>
#include <QStandardItemModel>
#include <QTimer>
#include <QTableWidget>
#include <QHostAddress>
#include <QNetworkInterface>

#include <QDebug>

#include "lamp/qcw_indicatorlamp.h"
#include"fanmotor/qmotor.h"
#include "userui/mplotui.h"
#include "userui/homewindow.h"
#include "dialogs/settingdialog.h"
#include "mainwindow/mainwindow.h"
#include "canui.h"
#include "dialogs/siglemotordialog.h"

//! \brief Tcp user define protocol (B : byte, s : start, add : address, n : number) :
/*!
 *! Fuction code(1 B): 0x03(read), 0x10(write), 0x2b(command)
 *!
 *! Read PDU: fuction code(0x03) + motor s add(2 B) + motor n(2 B) + \
 *!           register s add(2 B) + register n(2 B)
 *! Read response PDU: fuction code(0x03) + motor n(2 B) + register n(2 B) + \
 *!      motor1(register n * 2B) + motor2 + ...
 *! Error response PDU: fuction code(0x83) + error code(1 B)
 *!
 *! Write PDU: fuction code(0x10) + motor s add(2 B) + motor n(2 B) + \
 *!            register s add(2 B) + register n(2 B) + motor1(register n * 2B) + motor2 + ...
 *! write response PDU: fuction code(0x10) + motor n(2 B) + register n(2 B)
 *! Error response PDU: fuction code(0x90) + error code(1 B)
 *!
 *! Command PDU: fuction code(0x2b) + command code(1 B) + extra data(2 B)
 *! Command response PDU: fuction code(0x2b) + command code(1 B)
 *! Error response PDU: fuction code(0xab) + error code(1 B)
 *!
 *! Command code(1 B):
 *!
 *!
 *! Error code(1 B):
 *!
 */



//! \brief g_responseTimeout for modbus device request timeout set
const int g_responseTimeout = 50;

//! \brief g_numberOfTry for modbus device request number of try
const int g_numberOfTry = 2;

FanMotorUi *FanMotorUi::s_Instance = Q_NULLPTR;

//! CAN comunication fuction part start *********************************

/*!
 * \brief FanMotorUi::heartbeatError
 * \param d
 * \param heartbeatID
 */
void FanMotorUi::heartbeatError(CO_Data *d, unsigned char heartbeatID)
{
    UNS8 __address = heartbeatID - 1;

    m_motors->at(__address)->m_communicationState =FanCommunicationState::m_disconnect;

    m_motors->at(__address)->update();

    qDebug()<<d->nodeState<<"heartbeatError"<<heartbeatID;
}

/*!
 * \brief master_heartbeatError
 * \param d
 * \param heartbeatID
 */
void master_heartbeatError(CO_Data* d, UNS8 heartbeatID)
{
    FanMotorUi::getS_Instance()->heartbeatError(d, heartbeatID);

}

/*!
 * \brief master_post_sync
 * \param d
 */
void master_post_sync(CO_Data* d)
{
    qDebug()<<d->nodeState;
}

/*!
 * \brief FanMotorUi::post_SlaveStateChange
 * \param d
 * \param nodeId
 * \param newNodeState
 */
void FanMotorUi::post_SlaveStateChange(CO_Data* d, UNS8 nodeId, e_nodeState newNodeState)
{
    UNS8 __address = nodeId - 1;
    if(newNodeState == Disconnected){
        m_motors->at(__address)->m_communicationState =FanCommunicationState::m_disconnect;
    }
    else{
        m_motors->at(__address)->m_communicationState =FanCommunicationState::m_connect;
    }

    m_motors->at(__address)->update();

    if(nodeId == m_realTimeServerAddress){
    }

    qDebug()<<d->nodeState<<"post_SlaveStateChange"<<nodeId<<newNodeState;
}

/*!
 * \brief master_post_SlaveStateChange
 * \param d
 * \param nodeId
 * \param newNodeState
 */
void master_post_SlaveStateChange(CO_Data* d, UNS8 nodeId, e_nodeState newNodeState)
{
    FanMotorUi::getS_Instance()->post_SlaveStateChange(d,nodeId,newNodeState);

}

/*!
 * \brief OnMotoRealtimeDataUpdate
 * \param d
 * \param unsused_indextable
 * \param unsused_bSubindex
 * \return
 */
UNS32 OnMotoRealtimeDataUpdate(CO_Data* d, const indextable * unsused_indextable, UNS8 unsused_bSubindex)
{
    qDebug()<<d->nodeState<<"OnMotoRealtimeDataUpdate"<<unsused_indextable->bSubCount<<unsused_bSubindex;
    return 0;
}

/*!
 * \brief OnMotorParaUpdate
 * \param d
 * \param unsused_indextable
 * \param unsused_bSubindex
 * \return
 */
UNS32 OnMotorParaUpdate(CO_Data* d, const indextable * unsused_indextable, UNS8 unsused_bSubindex)
{
    qDebug()<<d->nodeState<<"OnMotorParaUpdate"<<unsused_indextable->bSubCount<<unsused_bSubindex;
    return 0;
}

/*!
 * \brief FanMotorUi::messageShow
 *        From can receive thread
 * \param s
 */
void FanMotorUi::messageShow(const QString &s)
{
    ui->textBrowser->append(s);
}

/*!
 * \brief FanMotorUi::messageHandle
 *        From can receive thread for frame handle
 * \param frame
 */
void FanMotorUi::messageHandle(const QCanBusFrame &frame)
{
    const qint8 dataLength = frame.payload().size();
    const qint32 id = frame.frameId();

    QString view;
    view += QLatin1String("Id: ");
    view += QString::number(id, 16);
    view += QLatin1String(" bytes: ");
    view += QString::number(dataLength, 10);
    view += QLatin1String(" data: ");
    view += frame.payload().toHex();

    ui->textBrowser->append(view);
}

//! CAN comunication fuction part end *********************************


/*!
 * \brief FanMotorUi::FanMotorUi constructor fuction
 * \param parent
 */
FanMotorUi::FanMotorUi(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FanMotorUi)
  , m_groups(Q_NULLPTR)
  , modbusRtuDevice(Q_NULLPTR)
  , m_motors(Q_NULLPTR)
  , m_masterBoard{new s_BOARD{}}
  , m_canThread{CanThread::getInstance()}
  , m_currentGroup(Q_NULLPTR)
  , modbusTcpDevice(Q_NULLPTR)
  , modbusTcpServer(Q_NULLPTR)
{
    ui->setupUi(this);
    qDebug("FanMotorUi init");

    //! Init fan data in canopen dictionary and register callback fuction
    {
    UNS32 sourceData = 0x20000110;
    UNS32 _dataSize = sizeof(UNS32);
    writeLocalDict(&master_Data, 0x1600, 1, &sourceData, &_dataSize, 1 );
    sourceData = 0x20000210;
    writeLocalDict(&master_Data, 0x1600, 2, &sourceData, &_dataSize, 1 );
    sourceData = 0x20000310;
    writeLocalDict(&master_Data, 0x1600, 3, &sourceData, &_dataSize, 1 );
    sourceData = 0x20000410;
    writeLocalDict(&master_Data, 0x1600, 4, &sourceData, &_dataSize, 1 );

    sourceData = 0x20000510;
    writeLocalDict(&master_Data, 0x1601, 1, &sourceData, &_dataSize, 1 );
    sourceData = 0x20000510;
    writeLocalDict(&master_Data, 0x1601, 2, &sourceData, &_dataSize, 1 );
    sourceData = 0x20000710;
    writeLocalDict(&master_Data, 0x1601, 3, &sourceData, &_dataSize, 1 );
    sourceData = 0x20000810;
    writeLocalDict(&master_Data, 0x1601, 4, &sourceData, &_dataSize, 1 );

    sourceData = 0x20020110;
    writeLocalDict(&master_Data, 0x1602, 1, &sourceData, &_dataSize, 1 );
    sourceData = 0x20020210;
    writeLocalDict(&master_Data, 0x1602, 2, &sourceData, &_dataSize, 1 );
    sourceData = 0x20020310;
    writeLocalDict(&master_Data, 0x1602, 3, &sourceData, &_dataSize, 1 );
    sourceData = 0x20020410;
    writeLocalDict(&master_Data, 0x1602, 4, &sourceData, &_dataSize, 1 );

    RegisterSetODentryCallBack(&master_Data, 0x2000, 8, &OnMotoRealtimeDataUpdate);
    RegisterSetODentryCallBack(&master_Data, 0x2002, 4, &OnMotorParaUpdate);
    }

    m_motorButtons.push_back(ui->pushButton_fan1);
    m_motorButtons.push_back(ui->pushButton_fan2);
    m_motorButtons.push_back(ui->pushButton_fan3);
    m_motorButtons.push_back(ui->pushButton_fan4);
    m_motorButtons.push_back(ui->pushButton_fan5);
    m_motorButtons.push_back(ui->pushButton_fan6);
    m_motorButtons.push_back(ui->pushButton_fan7);
    m_motorButtons.push_back(ui->pushButton_fan8);
    m_motorButtons.push_back(ui->pushButton_fan9);
    m_motorButtons.push_back(ui->pushButton_fan10);
    m_motorButtons.push_back(ui->pushButton_fan11);
    m_motorButtons.push_back(ui->pushButton_fan12);
    m_motorButtons.push_back(ui->pushButton_fan13);
    m_motorButtons.push_back(ui->pushButton_fan14);
    m_motorButtons.push_back(ui->pushButton_fan15);
    for(int i = 0; i < 15; i++){//!< All fan buttons use one slot fuction
        connect(m_motorButtons.at(i), SIGNAL(clicked(bool)), this, SLOT(onFanButtonclicked()));
    }

    ui->comboBox_fanType->addItem(QStringLiteral("Type0"), 0);
    ui->comboBox_fanType->addItem(QStringLiteral("Type1"), 1);
    ui->comboBox_fanType->addItem(QStringLiteral("Type2"), 2);
    ui->comboBox_fanType->addItem(QStringLiteral("Type3"), 3);

    ui->comboBox_fanPressure->addItem(QStringLiteral("Pressure0"), 0);
    ui->comboBox_fanPressure->addItem(QStringLiteral("Pressure1"), 1);
    ui->comboBox_fanPressure->addItem(QStringLiteral("Pressure2"), 2);
    ui->comboBox_fanPressure->addItem(QStringLiteral("Pressure3"), 3);

    ui->comboBox_fanreGulation->addItem(QStringLiteral("Regulation0"), 0);
    ui->comboBox_fanreGulation->addItem(QStringLiteral("Regulation1"), 1);
    ui->comboBox_fanreGulation->addItem(QStringLiteral("Regulation2"), 2);
    ui->comboBox_fanreGulation->addItem(QStringLiteral("Regulation2"), 3);

    ui->comboBox_fanControlMode->addItem(QStringLiteral("Mode0"), 0);
    ui->comboBox_fanControlMode->addItem(QStringLiteral("Mode1"), 1);


    m_masterBoard->busname = const_cast<char *>("master");//!< Init can board information
    m_masterBoard->baudrate = const_cast<char *>("125000");

    m_communication = CommunicationMode::Init;
    m_CANopenStart = false;

    onStateChanged(0);//Reset commucation state

    m_startServerAddress = 1;

    connect(ui->pushButton_startMotor, SIGNAL(clicked(bool)), this, SLOT(onRunStateButtonClicked()));
    connect(ui->pushButton_startMotor_A, SIGNAL(clicked(bool)), this, SLOT(onRunStateButtonClicked()));
    connect(ui->pushButton_startMotor_G, SIGNAL(clicked(bool)), this, SLOT(onRunStateButtonClicked()));
    connect(ui->pushButton_stopMotor, SIGNAL(clicked(bool)), this, SLOT(onRunStateButtonClicked()));
    connect(ui->pushButton_stopMotor_A, SIGNAL(clicked(bool)), this, SLOT(onRunStateButtonClicked()));
    connect(ui->pushButton_stopMotor_G, SIGNAL(clicked(bool)), this, SLOT(onRunStateButtonClicked()));

//    MPlotUi*__mPlotUi = MPlotUi::getInstance();
//    connect(this, &FanMotorUi::updatePlotUi, __mPlotUi, &MPlotUi::realtimeDataSlot);

    QToolButton *_openButton = MainWindow::getInstance()->getOpenButton();
    connect(_openButton, SIGNAL(clicked(bool)), ui->connectButton, SLOT(click()));
    QToolButton *_closeButton = MainWindow::getInstance()->getCloseButton();
    connect(_closeButton, SIGNAL(clicked(bool)), ui->disconnectButton, SLOT(click()));

    ui->textBrowser->document ()->setMaximumBlockCount (50);//!< Set textBrowser max block count

    homewindow * _homewindow = homewindow::getInstance();
    connect(_homewindow, &homewindow::readMotorRegister, this, &FanMotorUi::readFromMotor);
    connect(_homewindow, &homewindow::writeMotorRegister, this, &FanMotorUi::writeToMotor);

    connect(this, &FanMotorUi::writeMotorRegister, _homewindow, &homewindow::writeToMotor);


}

/*!
 * \brief Connect to seleted device
 */
void FanMotorUi::on_connectButton_clicked()
{
    //! Modbus Tcp connect
    if(ui->radioButton_tcp->isChecked()){
        QString _hostAddressString;
        quint16 _port;

        //![1] Read settings from file: "/SuperConsole.ini"
        QSettings settings(QDir::currentPath() + "/SuperConsole.ini", QSettings::IniFormat);
        settings.beginGroup("TcpPortSettings");
        _hostAddressString = settings.value("HostAddress", "127.0.0.1").toString();
        _port = settings.value("Port", "6474").toUInt();
        settings.endGroup();

        //![2] Make sure modbus device was not created yet
        if (modbusTcpDevice) {
            modbusTcpDevice->disconnectDevice();
            delete modbusTcpDevice;
            modbusTcpDevice = Q_NULLPTR;
        }

        //![3] Create modbus tcp client device and monitor error,state
        modbusTcpDevice = new QModbusTcpClient(this);

        connect(modbusTcpDevice, &QModbusClient::errorOccurred, [this](QModbusDevice::Error) {
            statusBar()->showMessage(modbusTcpDevice->errorString(), 3000);
        });

        if (!modbusTcpDevice) {
            statusBar()->showMessage(tr("Could not create Modbus client."), 3000);
            return;
        } else {
            connect(modbusTcpDevice, &QModbusClient::stateChanged,
                    this, &FanMotorUi::onStateChanged);//!< Monitor modbus device state
        }

        statusBar()->clearMessage();//!< Clear status bar first, we will use it

        //![4] Set port parameter and connect device
        if (modbusTcpDevice->state() != QModbusDevice::ConnectedState) {
            modbusTcpDevice->setConnectionParameter(QModbusDevice::NetworkPortParameter, _port);
            modbusTcpDevice->setConnectionParameter(QModbusDevice::NetworkAddressParameter, _hostAddressString);

            modbusTcpDevice->setTimeout(100);//!< Set response timeout 100ms
            modbusTcpDevice->setNumberOfRetries(2);//!< Set retry number 2

            //! Now try connect modbus device
            if (!modbusTcpDevice->connectDevice()) {//!< Connect failed
                statusBar()->showMessage(tr("Modbus connect failed: ") + modbusTcpDevice->errorString(), 3000);
            } else {//!< Connect  successfully

            }
        }
    }

    //! CAN connect
    if(ui->radioButton_can->isChecked()){

        CANUi *_canUi = CANUi::getS_Instance();
        _canUi->resetCAN();

        if(!canOpen(m_masterBoard, &master_Data)){
            ui->textBrowser->append(tr("CAN connect failed !"));
            statusBar()->showMessage(tr("CAN connect failed !"), 3000);
            return;
        }

        connect(m_canThread, SIGNAL(message(QString)),
                this, SLOT(messageShow(QString)));

        connect(m_canThread, SIGNAL(message(QCanBusFrame)),
                this, SLOT(messageHandle(QCanBusFrame)));

        m_canThread->mStart(true);//!< open canport and canopen receive thread

        master_Data.heartbeatError = master_heartbeatError;
        master_Data.post_sync = master_post_sync;

        master_Data.post_SlaveStateChange = master_post_SlaveStateChange;

        for(int i = 1; i <= m_motorNum; i++){//!< Heartbeat

            {//! 0x1016 :   Consumer Heartbeat Time  nodeid(bit 23 : 16) + time(bit 15 : 0) ms
                UNS32 _sourceData = 1100 + (i << 16);
                UNS32 _dataSize = sizeof(UNS32);
                if(OD_SUCCESSFUL != writeLocalDict(&master_Data, (UNS16)0x1016, (UNS8)i, (UNS32*)&_sourceData, &_dataSize, 1 )){
                    ui->textBrowser->append(tr("writeLocalDict 0x1016 %1: fail !").arg(i));
                }
            }

        }

        TimerInit();
        StartTimerLoop(&InitNodes);

        _canUi->setEnabled(false);//!< disable canui

        onStateChanged(1);
    }

    //! Modbus RTU connect
    if(ui->radioButton_modbus->isChecked()){
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
            statusBar()->showMessage(modbusRtuDevice->errorString(), 3000);
        });
        if (!modbusRtuDevice) {
            statusBar()->showMessage(tr("Could not create Modbus client."), 3000);
            return;
        } else {
            connect(modbusRtuDevice, &QModbusClient::stateChanged,
                    this, &FanMotorUi::onStateChanged);//!< Monitor modbus device state
        }

        statusBar()->clearMessage();//!< Clear status bar first, we will use it

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
                statusBar()->showMessage(tr("Modbus connect failed: ") + modbusRtuDevice->errorString(), 3000);
            } else {//!< Connect  successfully
            }
        }
    }

}

/*!
 * \brief Disconnect current connected device
 */
void FanMotorUi::on_disconnectButton_clicked()//! close
{
    //! Modbus Tcp disconnect
    if(m_communication == CommunicationMode::Tcp){

        if(!modbusTcpDevice)
            return;

        if(modbusTcpDevice->state() == QModbusDevice::ConnectedState){
            modbusTcpDevice->disconnectDevice();
        }
    }

    //! CAN disconnect
    if(m_communication == CommunicationMode::CANbus){
        if(m_CANopenStart){

        }

        m_canThread->mStop();               //!< then close thread
        disconnect(m_canThread, SIGNAL(message(QString)),
                this, SLOT(messageShow(QString)));

        disconnect(m_canThread, SIGNAL(message(QCanBusFrame)),
                this, SLOT(messageHandle(QCanBusFrame)));

        StopTimerLoop(&Exit);

        canClose(&master_Data);            //!< final close canport

        CANUi *_canUi = CANUi::getS_Instance();
        _canUi->setEnabled(true);
        onStateChanged(0);

    }

    //! Modbus RTU disconnect
    if(m_communication == CommunicationMode::Modbus){
        if(!modbusRtuDevice)
            return;
        if(modbusRtuDevice->state() == QModbusDevice::ConnectedState){
            modbusRtuDevice->disconnectDevice();
        }
    }

    if(ui->checkBox_monitorASW->isChecked())
        ui->checkBox_monitorASW->setChecked(false);

}

/*!
 * \brief Current device state changed  slot fuction
 * \param state
 */
void FanMotorUi::onStateChanged(int state)
{
    bool connected = (state != 0);

    //! Modbus RTU state change
    if(ui->radioButton_modbus->isChecked()){
        connected = (state == QModbusDevice::ConnectedState);
        if(connected){
            m_communication = CommunicationMode::Modbus;
            statusBar()->showMessage(tr("Modbus RTU connected !"), 3000);
        }
        else{
            m_communication = CommunicationMode::Init;
            statusBar()->showMessage(tr("Modbus RTU disonnected !"), 3000);
        }
    }

    //! CAN state change
    if(ui->radioButton_can->isChecked()){
        connected = (state != 0);
        if(connected){
            m_communication = CommunicationMode::CANbus;
            statusBar()->showMessage(tr("CAN connected !"), 5000);
        }
        else{
            m_communication = CommunicationMode::Init;
            statusBar()->showMessage(tr("CAN disonnected !"), 5000);
        }
    }

    //! Tcp state change
    if(ui->radioButton_tcp->isChecked()){
        connected = (state == QModbusDevice::ConnectedState);
        if(state == QModbusDevice::ConnectedState){
            m_communication = CommunicationMode::Tcp;

            //!If tcp client connected, tcp server connect
            if (modbusTcpServer) {
                modbusTcpServer->disconnectDevice();
                delete modbusTcpServer;
                modbusTcpServer = Q_NULLPTR;
            }

            modbusTcpServer = new HModbusTcpServer(this);

            connect(modbusTcpServer, &QModbusServer::errorOccurred, [this](QModbusDevice::Error) {
                ui->textBrowser->append(modbusTcpServer->errorString());
            });

            if (!modbusTcpServer) {
                ui->textBrowser->append(tr("Could not create Modbus Server."));
                return;
            } else {
                connect(modbusTcpServer, &QModbusServer::stateChanged,
                        this, &FanMotorUi::onTcpServerStateChanged);//!< Monitor modbus device state
            }

            //![1] Read settings from file: "/SuperConsole.ini"
            QSettings settings(QDir::currentPath() + "/SuperConsole.ini", QSettings::IniFormat);
            settings.beginGroup("TcpPortSettings");
            QString _hostAddressString = settings.value("HostAddress", "127.0.0.1").toString();
            quint16 _port = settings.value("Port", "6474").toUInt();
            settings.endGroup();

//            QString ipAddress;
//            QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
//            //! use the first non-localhost IPv4 address
//            for (int i = 0; i < ipAddressesList.size(); ++i) {
//                if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
//                    ipAddressesList.at(i).toIPv4Address() && ipAddressesList.at(i).isInSubnet(_hostAddressString, 24)) {
//                    ipAddress = ipAddressesList.at(i).toString();
//                    qDebug()<<ipAddress;
//                    break;
//                }
//            }
//            //! if we did not find one, use IPv4 localhost
//            if (ipAddress.isEmpty())
//                ipAddress = QHostAddress(QHostAddress::LocalHost).toString();


            //! modbus server connect
            if (modbusTcpServer->state() != QModbusDevice::ConnectedState) {
                _port++;
                modbusTcpServer->setConnectionParameter(QModbusDevice::NetworkPortParameter, 6475);
                modbusTcpServer->setConnectionParameter(QModbusDevice::NetworkAddressParameter,  "172.27.35.1");

                modbusTcpServer->setServerAddress(1);
                //! Now try connect modbus server device
                if (!modbusTcpServer->connectDevice()) {//!< Connect failed
                    ui->textBrowser->append(tr("Modbus connect failed: ") + modbusTcpServer->errorString());
                } else {//!< Connect  successfully
                }
            }

            statusBar()->showMessage(tr("Modbus Tcp connected !"), 5000);
        }
        else if(state == QModbusDevice::ConnectingState){
            m_communication = CommunicationMode::Init;
            statusBar()->showMessage(tr("Modbus Tcp connecting ... !"), 15000);
        }
        else{
            m_communication = CommunicationMode::Init;

            if(modbusTcpServer && modbusTcpServer->state() == QModbusDevice::ConnectedState){
                modbusTcpServer->disconnectDevice();
            }
            statusBar()->showMessage(tr("Modbus Tcp disonnected !"), 5000);
        }
    }

    ui->connectButton->setEnabled(!connected);
    ui->disconnectButton->setEnabled(connected);
    ui->groupBox_comState->setEnabled(!connected);

    ui->checkBox_monitorASW->setEnabled(connected);
    ui->pushButton_InitRead->setEnabled(connected);
    ui->pushButton_InitSetA->setEnabled(connected);
    ui->pushButton_InitSetG->setEnabled(connected);
    ui->pushButton_InitSetF->setEnabled(connected);
    ui->initializeAButton->setEnabled(connected);
    ui->initializeGButton->setEnabled(connected);
    ui->initializeButton->setEnabled(connected);
    ui->pushButton_startMotor_A->setEnabled(connected);
    ui->pushButton_startMotor_G->setEnabled(connected);
    ui->pushButton_startMotor->setEnabled(connected);
    ui->pushButton_stopMotor_A->setEnabled(connected);
    ui->pushButton_stopMotor_G->setEnabled(connected);
    ui->pushButton_stopMotor->setEnabled(connected);

    MainWindow::getInstance()->setStatusBarLamp(connected);

    emit updateSigleMotorState(connected);
}

void FanMotorUi::onTcpServerStateChanged(int state)
{
    if(state == QModbusDevice::ConnectedState){
        ui->textBrowser->append(tr("Modbus Tcp server connected !"));

        FCommandRegister fcr{FmotorCommand::m_connectToServer , 0, 0};
        sendCommand(fcr);
    }
    else if(state == QModbusDevice::ConnectingState){
        ui->textBrowser->append(tr("Modbus Tcp server connecting ... !"));
    }
    else if(state == QModbusDevice::UnconnectedState){
        ui->textBrowser->append(tr("Modbus Tcp server disonnected !"));
    }
    else{
        ui->textBrowser->append(tr("Modbus Tcp server closing ... !"));
    }
}

/*!
 * \brief FanMotorUi::readFromMotor  local modbus RTU master send read request to slave
 * \param motorAdd
 * \param registerAdd
 * \param count
 */
void FanMotorUi::readFromMotor(quint16 motorAdd, quint16 registerAdd, quint16 count)
{
    qDebug() <<"Read   entries";
    if(m_communication == CommunicationMode::Modbus){
        qDebug() <<"Read   entries1";
        if (!modbusRtuDevice)
            return;
        qDebug() <<"Read   entries3";

        int startAddress = registerAdd; //!< register address
        int numberOfEntries = count; //!< 16bit register number
        qDebug() <<"Read number Of entries:"<<numberOfEntries;
        QModbusDataUnit _modbusDataUnit(QModbusDataUnit::HoldingRegisters, startAddress, numberOfEntries);

        if (auto *reply = modbusRtuDevice->sendReadRequest(_modbusDataUnit, motorAdd)) {
            if (!reply->isFinished()){
                if(registerAdd == g_mSettingsRegisterAddress)
                    connect(reply, &QModbusReply::finished, this, &FanMotorUi::readInitReady);
                else if(registerAdd == g_mRealTimeRegisterAddress)
                    connect(reply, &QModbusReply::finished, this, &FanMotorUi::monitorReadReady);
                else if(registerAdd == g_mControllerRegisterAddress)
                    connect(reply, &QModbusReply::finished, this, &FanMotorUi::readInitFGAReady);
                else if(registerAdd == g_mPIParaRegisterAddress)
                    connect(reply, &QModbusReply::finished, this, &FanMotorUi::readPIReady);
            }
            else
                delete reply; //!< broadcast replies return immediately
        } else {
            ui->textBrowser->append(tr("Read error: ") + modbusRtuDevice->errorString());
        }
    }

}

/*!
 * \brief FanMotorUi::writeToMotor  local modbus RTU master send write request to slave
 * \param motorAdd
 * \param registerAdd
 * \param count
 */
void FanMotorUi::writeToMotor(quint16 motorAdd, quint16 registerAdd, quint16 count)
{
    if(m_communication == CommunicationMode::Modbus){
        if (!modbusRtuDevice)
            return;

        //![1] Init data
        int _address = motorAdd;

        QMotor *motor;
        foreach(FanGroupInfo *_group, *m_groups){//!< Get corresponding motor
            //! Judge belong to which group
            if(motorAdd >= _group->m_startAddress && \
                    motorAdd < _group->m_startAddress + _group->m_motors.count())
            {
                foreach(QMotor *_motor, _group->m_motors){
                    if(_motor->m_address == motorAdd){
                        motor = _motor;
                        break;
                    }
                }
            }
        }
        if(!motor){
            return;
        }
        quint16 *buff = (quint16 *)&motor->m_initSetttings;

        //![2] Pack data to a QModbusDataUnit
        const QModbusDataUnit::RegisterType table = QModbusDataUnit::HoldingRegisters;
        int startAddress = registerAdd;
        int numberOfEntries = count;
        QModbusDataUnit writeUnit = QModbusDataUnit(table, startAddress, numberOfEntries);
        for (uint i = 0; i < writeUnit.valueCount(); i++) {

            writeUnit.setValue(i, *buff++);
        }

        //![3] Send write request to node(adress:)
        if (auto *reply = modbusRtuDevice->sendWriteRequest(writeUnit, _address)) {
            if (!reply->isFinished()) {
                connect(reply, &QModbusReply::finished, this, [this, reply]() {
                    if (reply->error() == QModbusDevice::ProtocolError) {
                        ui->textBrowser->append(tr("Write response error: %1 (Mobus exception: 0x%2)")
                                                .arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 16)
                                                );
                    } else if (reply->error() != QModbusDevice::NoError) {
                        ui->textBrowser->append(tr("Write response error: %1 (code: 0x%2)").
                                                arg(reply->errorString()).arg(reply->error(), -1, 16));
                    }
                    reply->deleteLater();
                });
            } else {//!< Broadcast replies return immediately
                reply->deleteLater();
            }
        } else {
            ui->textBrowser->append(tr("Write error: ") + modbusRtuDevice->errorString());
        }
    }

}

/*!
 * \brief FanMotorUi::on_spinBox_motorNum_valueChanged  slot fuction
 *        Change current group motor number
 * \param arg1
 */
void FanMotorUi::on_spinBox_motorNum_valueChanged(int arg1)
{
    m_motorNum = arg1;
    if(!m_motors)
        return;

    while(m_motors->count() < m_motorNum){
        int __motorCount = m_motors->count();
        m_motors->push_back(new QMotor(__motorCount + m_currentGroup->m_startAddress));
    }

    while(m_motors->count() > m_motorNum){
        m_motors->pop_back();
    }

    for(int i = arg1; i < m_motorButtons.count(); i++)//!< For reduce group
    {
        m_motorButtons.at(i)->setEnabled(false);
    }
    for(int i = 0; i < arg1; i++)        //!< For add group
    {
        m_motorButtons.at(i)->setEnabled(true);
    }

    if(m_currentGroup)//!< Update max address at current address
        ui->spinBox_InitAdress->setMaximum(m_currentGroup->m_startAddress + m_motorNum - 1);
}

/*!
 * \brief FanMotorUi::on_settingsButton_clicked  slot fuction
 */
void FanMotorUi::on_settingsButton_clicked()
{
    SettingDialog* settingDialog = new SettingDialog();
    settingDialog->exec();//!< exec: if we don't close dialog ,we can't operate other window
}

/*!
 * \brief FanMotorUi::getS_Instance
 * \return
 */
FanMotorUi *FanMotorUi::getS_Instance()
{
    if(!s_Instance)
    {
        s_Instance = new FanMotorUi;
    }
    return s_Instance;
}

/*!
 * \brief FanMotorUi::deleteInstance
 */
void FanMotorUi::deleteInstance()
{
    if(s_Instance)
        s_Instance->deleteLater();
}

/*!
 * \brief FanMotorUi::~FanMotorUi
 */
FanMotorUi::~FanMotorUi()
{
    //! If modbus rtu have created , disconnect it and delete
    if(modbusRtuDevice){
        disconnect(modbusRtuDevice, &QModbusClient::stateChanged,
                this, &FanMotorUi::onStateChanged);
        if(modbusRtuDevice->state() != QModbusDevice::UnconnectedState)
            modbusRtuDevice->disconnectDevice();
        modbusRtuDevice->deleteLater();
    }

    //! If modbus tcp have created , disconnect it and delete
    if(modbusTcpDevice){
        disconnect(modbusTcpDevice, &QModbusClient::stateChanged,
                this, &FanMotorUi::onStateChanged);
        if(modbusTcpDevice->state() != QModbusDevice::UnconnectedState){
            modbusTcpDevice->disconnectDevice();
        }
        modbusTcpDevice->deleteLater();
    }

    //! Delete static SigleMotorDialog
    SigleMotorDialog::deleteInstance();

    s_Instance = Q_NULLPTR;
    delete ui;
    qDebug()<<"FanMotorUi exit";
}

/*!
 * \brief Get mainwindow's statusBar
 * \return QStatusBar
 */
QStatusBar* FanMotorUi::statusBar()
{
    return MainWindow::getInstance()->getStatusBar();
}

/*!
 * \brief Change a group and show group window
 * \param group
 */
void FanMotorUi::changeGroup(FanGroupInfo *group)
{
    if(m_currentGroup == group)//!< If the same group, do nothing
        return;

    //! Get group parameter
    if(!m_groups)
        m_groups = homewindow::getInstance()->groups();
    m_currentGroup = group; //!< Get group pointer
    m_motors = &m_currentGroup->m_motors; //!< Get motor container pointer
    int _startAddress = m_currentGroup->m_startAddress; //!< Get current group start address
    int _fanMaxNumber = m_currentGroup->m_fanMaxNumber; //!< Get current group
    int _fanNumber = m_motors->count(); //!< Get current group motor number

    //! If current group motor number is 0, we reset it to maxNumber
    if(m_motors->count() == 0){
        _fanNumber = _fanMaxNumber;
        for(int i = 0; i< _fanNumber; i++){ //!< Init table
            m_motors->push_back(new QMotor(i+_startAddress));
        }
    }

    //! Update widget
    ui->lcdNumber_groupNum->display(m_currentGroup->m_groupID);
    ui->spinBox_motorNum->setValue(_fanNumber); //!< Motor number spinBox set the right number
    ui->spinBox_motorNum->setMaximum(_fanMaxNumber); //!< Motor number spinBox set maxNumber
    ui->spinBox_InitAdress->setMinimum(_startAddress); //!< SpinBox_InitAdress for first fan in current group
    ui->spinBox_InitAdress->setMaximum(_startAddress + _fanNumber - 1); //!< Last address in current group


}

/* Init parameter set and read fuction part start *********************************/

/*!
 * \brief Set one fan Init parameter  slot fuction
 */
void FanMotorUi::on_pushButton_InitSetF_clicked()
{
    if(m_communication == CommunicationMode::Modbus){
        if (!modbusRtuDevice)
            return;

        //![1] Init data
        QAbstractItemModel *__model = ui->table_settings->model();
        int _address = ui->spinBox_InitAdress->value();//Server address
        QMotor motor(0);
        quint16 *buff = (quint16 *)&motor.m_initSetttings;
        //![2] Get data from tableview
        for(unsigned char i = 0; i<3; i++){
            for(unsigned char j = 1; j<5; j++){
                *buff = (quint16)(__model->data(__model->index(i, j)).toDouble() * 100);
                buff++;
            }
        }
        int _fanType = ui->comboBox_fanType->currentIndex();
        int _fanPressure = ui->comboBox_fanPressure->currentIndex();
        int _fanreGulation = ui->comboBox_fanreGulation->currentIndex();
        int _fanControlMode = ui->comboBox_fanControlMode->currentIndex();
        *buff = (_fanType << 12) + (_fanPressure << 10) + (_fanreGulation << 8) + (_fanControlMode << 7);

        //![3] Pack data to a QModbusDataUnit
        buff = (quint16 *)&motor.m_initSetttings;
        const QModbusDataUnit::RegisterType table = QModbusDataUnit::HoldingRegisters;
        int startAddress = g_mSettingsRegisterAddress;
        int numberOfEntries = g_mSettingsRegisterCount;
        QModbusDataUnit writeUnit = QModbusDataUnit(table, startAddress, numberOfEntries);
        for (uint i = 0; i < writeUnit.valueCount(); i++) {

            writeUnit.setValue(i, *buff++);
        }
        qDebug() <<writeUnit.valueCount();

        //![4] Send write request to node(adress:)
        if (auto *reply = modbusRtuDevice->sendWriteRequest(writeUnit, _address)) {
            if (!reply->isFinished()) {
                connect(reply, &QModbusReply::finished, this, [this, reply]() {
                    if (reply->error() == QModbusDevice::ProtocolError) {
                        ui->textBrowser->append(tr("Write response error: %1 (Mobus exception: 0x%2)")
                                                .arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 16)
                                                );
                    } else if (reply->error() != QModbusDevice::NoError) {
                        ui->textBrowser->append(tr("Write response error: %1 (code: 0x%2)").
                                                arg(reply->errorString()).arg(reply->error(), -1, 16));
                    }
                    reply->deleteLater();
                });
            } else {//!< Broadcast replies return immediately
                reply->deleteLater();
            }
        } else {
            ui->textBrowser->append(tr("Write error: ") + modbusRtuDevice->errorString());
        }
    }

    if(m_communication == CommunicationMode::Tcp){
        if (!modbusTcpDevice)
            return;

        //![1] Init data
        QAbstractItemModel *__model = ui->table_settings->model();
        int _address = ui->spinBox_InitAdress->value();//!< Motor address
        QMotor motor(0);
        quint16 *buff = (quint16 *)&motor.m_initSetttings;

        //![2] Get data from tableview
        for(unsigned char i = 0; i<3; i++){
            for(unsigned char j = 1; j<5; j++){
                *buff = (quint16)(__model->data(__model->index(i, j)).toDouble() * 100);
                buff++;
            }
        }
        int _fanType = ui->comboBox_fanType->currentIndex();
        int _fanPressure = ui->comboBox_fanPressure->currentIndex();
        int _fanreGulation = ui->comboBox_fanreGulation->currentIndex();
        int _fanControlMode = ui->comboBox_fanControlMode->currentIndex();
        *buff = (_fanType << 12) + (_fanPressure << 10) + (_fanreGulation << 8) + (_fanControlMode << 7);

        //![3] Pack data to a QModbusDataUnit
        buff = (quint16 *)&motor.m_initSetttings;
        const QModbusDataUnit::RegisterType table = QModbusDataUnit::HoldingRegisters;

        //! 0:15 register address , 16:31 motor address
        int startAddress = (g_mSettingsRegisterAddress & 0x00ff) | ((_address << 8) & 0xff00);
        int numberOfEntries = g_mSettingsRegisterCount;
        QModbusDataUnit writeUnit = QModbusDataUnit(table, startAddress, numberOfEntries);
        for (uint i = 0; i < writeUnit.valueCount(); i++) {

            writeUnit.setValue(i, *buff++);
        }
        qDebug() <<writeUnit.valueCount();

        //![4] Send write request to tcp server (adress:)
        int tcpServerAddress = 1;
        if (auto *reply = modbusTcpDevice->sendWriteRequest(writeUnit, tcpServerAddress)) {
            if (!reply->isFinished()) {
                connect(reply, &QModbusReply::finished, this, [this, reply]() {
                    if (reply->error() == QModbusDevice::ProtocolError) {
                        ui->textBrowser->append(tr("Write response error: %1 (Mobus exception: 0x%2)")
                                                .arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 16)
                                                );
                    } else if (reply->error() != QModbusDevice::NoError) {
                        ui->textBrowser->append(tr("Write response error: %1 (code: 0x%2)").
                                                arg(reply->errorString()).arg(reply->error(), -1, 16));
                    }
                    reply->deleteLater();
                });
            } else {//!< Broadcast replies return immediately
                reply->deleteLater();
            }
        } else {
            ui->textBrowser->append(tr("Write error: ") + modbusTcpDevice->errorString());
        }
    }

}

/*!
 * \brief Set group fan Init parameter  slot fuction
 */
void FanMotorUi::on_pushButton_InitSetG_clicked()
{
    if(m_communication == CommunicationMode::Modbus){
        if (!modbusRtuDevice)
            return;

        //![1] Init data
        QAbstractItemModel *__model = ui->table_settings->model();
        int _startAddress = ui->spinBox_InitAdress->minimum();//!< Server address
        int _lastAddress = ui->spinBox_InitAdress->maximum();
        QMotor motor(0);
        quint16 *buff = (quint16 *)&motor.m_initSetttings;

        //![2] Get data from tableview
        for(unsigned char i = 0; i<3; i++){
            for(unsigned char j = 1; j<5; j++){
                *buff = (quint16)(__model->data(__model->index(i, j)).toDouble() * 100);
                buff++;
            }
        }
        int _fanType = ui->comboBox_fanType->currentIndex();
        int _fanPressure = ui->comboBox_fanPressure->currentIndex();
        int _fanreGulation = ui->comboBox_fanreGulation->currentIndex();
        int _fanControlMode = ui->comboBox_fanControlMode->currentIndex();
        *buff = (_fanType << 12) + (_fanPressure << 10) + (_fanreGulation << 8) + (_fanControlMode << 7);


        //![3] Pack data to a QModbusDataUnit
        buff = (quint16 *)&motor.m_initSetttings;
        const QModbusDataUnit::RegisterType table = QModbusDataUnit::HoldingRegisters;
        int startAddress = g_mSettingsRegisterAddress;
        int numberOfEntries = g_mSettingsRegisterCount;
        QModbusDataUnit writeUnit = QModbusDataUnit(table, startAddress, numberOfEntries);
        for (uint i = 0; i < writeUnit.valueCount(); i++) {

            writeUnit.setValue(i, *buff++);
        }
        qDebug() <<writeUnit.valueCount();

        //![4] Send write request to node(adress:i) at current group
        for(int i = _startAddress; i <= _lastAddress; i++){

            if (auto *reply = modbusRtuDevice->sendWriteRequest(writeUnit, i)) {
                if (!reply->isFinished()) {
                    connect(reply, &QModbusReply::finished, this, [this, reply]() {
                        if (reply->error() == QModbusDevice::ProtocolError) {
                            ui->textBrowser->append(tr("Write response error: %1 (Mobus exception: 0x%2)")
                                                    .arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 16)
                                                    );
                        } else if (reply->error() != QModbusDevice::NoError) {
                            ui->textBrowser->append(tr("Write response error: %1 (code: 0x%2)").
                                                    arg(reply->errorString()).arg(reply->error(), -1, 16));
                        }
                        reply->deleteLater();
                    });
                } else {//!< Broadcast replies return immediately
                    reply->deleteLater();
                }
            } else {
                ui->textBrowser->append(tr("Write error: ") + modbusRtuDevice->errorString());
            }
        }
    }

    if(m_communication == CommunicationMode::Tcp){
        if (!modbusTcpDevice)
            return;

        //![1] Init data
        QAbstractItemModel *__model = ui->table_settings->model();
        int _startAddress = ui->spinBox_InitAdress->minimum();//!< Server address
        int _lastAddress = ui->spinBox_InitAdress->maximum();
        QMotor motor(0);
        quint16 *buff = (quint16 *)&motor.m_initSetttings;

        //![2] Get data from tableview
        for(unsigned char i = 0; i<3; i++){
            for(unsigned char j = 1; j<5; j++){
                *buff = (quint16)(__model->data(__model->index(i, j)).toDouble() * 100);
                buff++;
            }
        }
        int _fanType = ui->comboBox_fanType->currentIndex();
        int _fanPressure = ui->comboBox_fanPressure->currentIndex();
        int _fanreGulation = ui->comboBox_fanreGulation->currentIndex();
        int _fanControlMode = ui->comboBox_fanControlMode->currentIndex();
        *buff = (_fanType << 12) + (_fanPressure << 10) + (_fanreGulation << 8) + (_fanControlMode << 7);


        //![3] Pack data to a QModbusDataUnit
        buff = (quint16 *)&motor.m_initSetttings;
        const QModbusDataUnit::RegisterType table = QModbusDataUnit::HoldingRegisters;

        //! 0:15 register address , 16:31 motor address
        int startAddress = 0;
        int numberOfEntries = g_mSettingsRegisterCount;
        QModbusDataUnit writeUnit = QModbusDataUnit(table, startAddress, numberOfEntries);
        for (uint i = 0; i < writeUnit.valueCount(); i++) {

            writeUnit.setValue(i, *buff++);
        }
        qDebug() <<writeUnit.valueCount();

        //![4] Send write request to node(adress:i) at current group
        int tcpServerAddress = 1;
        for(int i = _startAddress; i <= _lastAddress; i++){

            //! 0:15 register address , 16:31 motor address
            startAddress = (g_mSettingsRegisterAddress & 0x00ff) | ((i << 8) & 0xff00);
            writeUnit.setStartAddress(startAddress);

            if (auto *reply = modbusTcpDevice->sendWriteRequest(writeUnit, tcpServerAddress)) {
                if (!reply->isFinished()) {
                    connect(reply, &QModbusReply::finished, this, [this, reply]() {
                        if (reply->error() == QModbusDevice::ProtocolError) {
                            ui->textBrowser->append(tr("Write response error: %1 (Mobus exception: 0x%2)")
                                                    .arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 16)
                                                    );
                        } else if (reply->error() != QModbusDevice::NoError) {
                            ui->textBrowser->append(tr("Write response error: %1 (code: 0x%2)").
                                                    arg(reply->errorString()).arg(reply->error(), -1, 16));
                        }
                        reply->deleteLater();
                    });
                } else {//!< Broadcast replies return immediately
                    reply->deleteLater();
                }
            } else {
                ui->textBrowser->append(tr("Write error: ") + modbusTcpDevice->errorString());
            }
        }
    }
}

/*!
 * \brief Set all fan Init parameter  slot fuction
 */
void FanMotorUi::on_pushButton_InitSetA_clicked()
{
    if(m_communication == CommunicationMode::Modbus){
        if (!modbusRtuDevice)
            return;

        //![1] Init data
        QAbstractItemModel *__model = ui->table_settings->model();
        int _address = 0;//!< Broadcast address
        QMotor motor(0);
        quint16 *buff = (quint16 *)&motor.m_initSetttings;

        //![2] Get data from tableview
        for(unsigned char i = 0; i<3; i++){
            for(unsigned char j = 1; j<5; j++){
                *buff = (quint16)(__model->data(__model->index(i, j)).toDouble() * 100);
                buff++;
            }
        }
        int _fanType = ui->comboBox_fanType->currentIndex();
        int _fanPressure = ui->comboBox_fanPressure->currentIndex();
        int _fanreGulation = ui->comboBox_fanreGulation->currentIndex();
        int _fanControlMode = ui->comboBox_fanControlMode->currentIndex();
        *buff = (_fanType << 12) + (_fanPressure << 10) + (_fanreGulation << 8) + (_fanControlMode << 7);

        //![3] Pack data to a QModbusDataUnit
        buff = (quint16 *)&motor.m_initSetttings;
        const QModbusDataUnit::RegisterType table = QModbusDataUnit::HoldingRegisters;
        int startAddress = g_mSettingsRegisterAddress;
        int numberOfEntries = g_mSettingsRegisterCount;
        QModbusDataUnit writeUnit = QModbusDataUnit(table, startAddress, numberOfEntries);
        for (uint i = 0; i < writeUnit.valueCount(); i++) {

            writeUnit.setValue(i, *buff++);
        }
        qDebug() <<writeUnit.valueCount();

        //![4] Send write request to node(adress:0) : Broadcast
        if (auto *reply = modbusRtuDevice->sendWriteRequest(writeUnit, _address)) {
            if (!reply->isFinished()) {
            } else {//!< Broadcast replies return immediately
                reply->deleteLater();
            }
        } else {
            ui->textBrowser->append(tr("Write error: ") + modbusRtuDevice->errorString());
        }
    }

    if(m_communication == CommunicationMode::Tcp){
        if (!modbusTcpDevice)
            return;

        //![1] Init data
        QAbstractItemModel *__model = ui->table_settings->model();
        int _address = 0;//!< Broadcast address
        QMotor motor(0);
        quint16 *buff = (quint16 *)&motor.m_initSetttings;

        //![2] Get data from tableview
        for(unsigned char i = 0; i<3; i++){
            for(unsigned char j = 1; j<5; j++){
                *buff = (quint16)(__model->data(__model->index(i, j)).toDouble() * 100);
                buff++;
            }
        }
        int _fanType = ui->comboBox_fanType->currentIndex();
        int _fanPressure = ui->comboBox_fanPressure->currentIndex();
        int _fanreGulation = ui->comboBox_fanreGulation->currentIndex();
        int _fanControlMode = ui->comboBox_fanControlMode->currentIndex();
        *buff = (_fanType << 12) + (_fanPressure << 10) + (_fanreGulation << 8) + (_fanControlMode << 7);

        //![3] Pack data to a QModbusDataUnit
        buff = (quint16 *)&motor.m_initSetttings;
        const QModbusDataUnit::RegisterType table = QModbusDataUnit::HoldingRegisters;

        //! 0:15 register address , 16:31 motor address
        int startAddress = (g_mSettingsRegisterAddress & 0x00ff) | ((_address << 8) & 0xff00);
        int numberOfEntries = g_mSettingsRegisterCount;
        QModbusDataUnit writeUnit = QModbusDataUnit(table, startAddress, numberOfEntries);
        for (uint i = 0; i < writeUnit.valueCount(); i++) {

            writeUnit.setValue(i, *buff++);
        }
        qDebug() <<writeUnit.valueCount();

        //![4] Send write request to tcp server (adress:)
        int tcpServerAddress = 1;
        if (auto *reply = modbusTcpDevice->sendWriteRequest(writeUnit, tcpServerAddress)) {
            if (!reply->isFinished()) {
                connect(reply, &QModbusReply::finished, this, [this, reply]() {
                    if (reply->error() == QModbusDevice::ProtocolError) {
                        ui->textBrowser->append(tr("Write response error: %1 (Mobus exception: 0x%2)")
                                                .arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 16)
                                                );
                    } else if (reply->error() != QModbusDevice::NoError) {
                        ui->textBrowser->append(tr("Write response error: %1 (code: 0x%2)").
                                                arg(reply->errorString()).arg(reply->error(), -1, 16));
                    }
                    reply->deleteLater();
                });
            } else {//!< Broadcast replies return immediately
                reply->deleteLater();
            }
        } else {
            ui->textBrowser->append(tr("Write error: ") + modbusTcpDevice->errorString());
        }
    }
}

/*!
 * \brief Read fan Init parameter  slot fuction
 */
void FanMotorUi::on_pushButton_InitRead_clicked()
{
    if(m_communication == CommunicationMode::Modbus){
        if (!modbusRtuDevice)
            return;

        int startAddress = g_mSettingsRegisterAddress; //register address
        int numberOfEntries = g_mSettingsRegisterCount; //16bit register number
        qDebug() <<"Read number Of entries:"<<numberOfEntries;
        QModbusDataUnit _modbusDataUnit(QModbusDataUnit::HoldingRegisters, startAddress, numberOfEntries);

        if (auto *reply = modbusRtuDevice->sendReadRequest(_modbusDataUnit, ui->spinBox_InitAdress->value())) {
            if (!reply->isFinished()){
                connect(reply, &QModbusReply::finished, this, &FanMotorUi::readInitReady);
            }
            else
                delete reply; // broadcast replies return immediately
        } else {
            ui->textBrowser->append(tr("Read error: ") + modbusRtuDevice->errorString());
        }
    }

    if(m_communication == CommunicationMode::Tcp){
        if (!modbusTcpDevice)
            return;

        int _address = ui->spinBox_InitAdress->value();//!< Motor address

        FCommandRegister fcr;
        fcr.m_command = FmotorCommand::m_readMotorRegister;
        //! 0:15 register address , 16:31 motor address
        fcr.m_registerAdd = (g_mSettingsRegisterAddress & 0x00ff) | ((_address << 8) & 0xff00);
        fcr.m_count = g_mSettingsRegisterCount;
        sendCommand(fcr);
    }
}

/*!
 * \brief Read fan Init parameter Ready  slot fuction
 */
void FanMotorUi::readInitReady()
{
    QModbusReply *reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;

    int __pos = ui->spinBox_InitAdress->value() - ui->spinBox_InitAdress->minimum();//!< Current motor pos of container

    //! Reply no error
    if (reply->error() == QModbusDevice::NoError) {

        const QModbusDataUnit unit = reply->result();//!< Get reply data unit

        quint16 *buff;
        if((unit.startAddress() & 0x00ff)== g_mSettingsRegisterAddress){//!< Make sure is Init data
            buff = (quint16 *)&m_motors->at(__pos)->m_initSetttings;//!< Get current motor's initSetttings pointer
        }

        //! Put data to current motor's initSetttings
        for (uint i = 0; i < unit.valueCount(); i++) {

            *buff++ = unit.value(i);

            const QString entry = tr("Address: %1, Value: %2").arg(QString::number(unit.startAddress()+i, 16))
                    .arg(QString::number(unit.value(i), 16));
            ui->textBrowser->append(entry);//!< Show out the data
        }

        m_motors->at(__pos)->m_communicationState =FanCommunicationState::m_connect;//Update communication state

    }
    //! Reply protocol error
    else if (reply->error() == QModbusDevice::ProtocolError) {
        ui->textBrowser->append(tr("Read response error: %1 (Mobus exception: 0x%2)").
                                arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 16));
        m_motors->at(__pos)->m_communicationState =FanCommunicationState::m_comError;//Update communication state
    }
    //Reply timeout
    else {
        ui->textBrowser->append(tr("Read response error: %1 (code: 0x%2)").
                                arg(reply->errorString()).arg(reply->error(), -1, 16));
        m_motors->at(__pos)->m_communicationState =FanCommunicationState::m_disconnect;//!< Update communication state
    }

    m_motors->at(__pos)->update();//!< Update motor data to ui

    //! Update data to settings table
    QAbstractItemModel *__model = ui->table_settings->model();
    quint16 *__buffPtr = (quint16 *)&m_motors->at(__pos)->m_initSetttings;
    for(unsigned char i = 0; i<3; i++){
        for(unsigned char j = 1; j<5; j++){
            __model->setData(__model->index(i,j), (double)(*__buffPtr)  * 0.01);
            __buffPtr++;
        }
    }

    int _fanType = (*__buffPtr >> 12) & 0x000f;
    ui->comboBox_fanType->setCurrentIndex(_fanType);
    int _fanPressure = (*__buffPtr >> 10) & 0x0003;
    ui->comboBox_fanPressure->setCurrentIndex(_fanPressure);
    int _fanreGulation = (*__buffPtr >> 8) & 0x0003;
    ui->comboBox_fanreGulation->setCurrentIndex(_fanreGulation);
    int _fanControlMode = (*__buffPtr >> 7) & 0x0001;
    ui->comboBox_fanControlMode->setCurrentIndex(_fanControlMode);

    //! For update sigle motor ui
    if(m_motors->at(__pos)->m_address == ui->spinBox_InitAdress->value()){
        if(!SigleMotorDialog::getS_Instance()->isHidden())
            emit updateSigleMotor(0);
    }

    if(m_communication == CommunicationMode::Modbus){
        emit writeMotorRegister(m_motors->at(__pos)->m_address, g_mSettingsRegisterAddress, g_mSettingsRegisterCount);
    }

    reply->deleteLater();
}

/* Init parameter set and read fuction part end ***********************************/



/* Group and all motor monitor fuction part start *********************************/

void FanMotorUi::on_groupmonitorButton_clicked()
{
    if(!m_currentGroup->m_monitorDialog)
        m_currentGroup->m_monitorDialog = new GroupMonitorDialog(&m_currentGroup->m_motors);

    m_currentGroup->m_monitorDialog->show(m_currentGroup->m_groupID, &m_currentGroup->m_motors);
}

/*!
 * \brief Open or close monitor all fan  slot fuction
 * \param arg1
 */
void FanMotorUi::on_checkBox_monitorASW_stateChanged(int arg1)
{
    //![1] Get all motor pointer
    if(!m_groups)
        m_groups = homewindow::getInstance()->groups();

    //![2] For for each motor change the monitor state
//    foreach(FanGroupInfo *group, *m_groups){
//        foreach(QMotor *_motor, group->m_motors){
//                _motor->isMonitor = arg1;
//        }
//    }

    //![3.1] For modbus rtu mode, turn to fuction "monitor_stateChanged"
    if(m_communication == CommunicationMode::Modbus){
        monitor_stateChanged(arg1);
    }
    //![3.2] For canopen mode
    else if(m_communication == CommunicationMode::CANbus){
        if(!m_CANopenStart){

            //pdo config
            {//0x1600 :   RPDO1 COB-ID 0x180 + nodeid
                UNS32 _sourceData = 0x180 + 1;
                UNS32 _dataSize = sizeof(UNS32);
                if(OD_SUCCESSFUL != writeLocalDict(&master_Data, (UNS16)0x1400, (UNS8)1, (UNS32*)&_sourceData, &_dataSize, 1 )){
                    ui->textBrowser->append("writeLocalDict 0x1400 fail !");
                }
            }
            {//0x1600 :   RPDO2 COB-ID 0x180 + nodeid
                UNS32 _sourceData = 0x280 + 1;
                UNS32 _dataSize = sizeof(UNS32);
                if(OD_SUCCESSFUL != writeLocalDict(&master_Data, (UNS16)0x1401, (UNS8)1, (UNS32*)&_sourceData, &_dataSize, 1 )){
                    ui->textBrowser->append("writeLocalDict 0x1400 fail !");
                }
            }

            //SYNC config
            {//0x1005 :   SYNC COB ID gen(bit 30) COB-ID : 80
                UNS32 _sourceData = 0x40000080;
                UNS32 _dataSize = sizeof(UNS32);
                if(OD_SUCCESSFUL != writeLocalDict(&master_Data, (UNS16)0x1005, (UNS8)0, &_sourceData, &_dataSize, 1 )){
                    ui->textBrowser->append("writeLocalDict 0x1005 fail !");
                }
            }

            {//0x1006 :   SYNC Cycle Period   us
                UNS32 _sourceData = 500000;
                UNS32 _dataSize = sizeof(UNS32);
                if(OD_SUCCESSFUL != writeLocalDict(&master_Data, (UNS16)0x1006, (UNS8)0, &_sourceData, &_dataSize, 1 )){
                    ui->textBrowser->append("writeLocalDict 0x1006 fail !");
                }
            }

            for(int i = 1; i <= m_motorNum; i++){

                {//0x1016 :   Consumer Heartbeat Time  nodeid(bit 23 : 16) + time(bit 15 : 0) ms
                    UNS32 _sourceData = 1100 + (i << 16);
                    UNS32 _dataSize = sizeof(UNS32);
                    if(OD_SUCCESSFUL != writeLocalDict(&master_Data, (UNS16)0x1016, (UNS8)i, (UNS32*)&_sourceData, &_dataSize, 1 )){
                        ui->textBrowser->append(tr("writeLocalDict 0x1016 %1: fail !").arg(i));
                    }
                }

                masterSendNMTstateChange (&master_Data, i, NMT_Start_Node);
            }

            m_CANopenStart = true;

        }
        else{

            for(int i = 1; i <= m_motorNum; i++){
                masterSendNMTstateChange (&master_Data, i, NMT_Stop_Node);
            }

            {//0x1005 :   SYNC COB ID gen(bit 30) COB-ID : 80
                UNS32 _sourceData = 0x00000080;
                UNS32 _dataSize = sizeof(UNS32);
                if(OD_SUCCESSFUL != writeLocalDict(&master_Data, (UNS16)0x1005, (UNS8)0, (UNS32*)&_sourceData, &_dataSize, 1 )){
                    ui->textBrowser->append("writeLocalDict 0x1005 fail !");
                }
            }

            {//0x1006 :   SYNC Cycle Period   us
                UNS32 _sourceData = 0;
                UNS32 _dataSize = sizeof(UNS32);
                if(OD_SUCCESSFUL != writeLocalDict(&master_Data, (UNS16)0x1006, (UNS8)0, (UNS32*)&_sourceData, &_dataSize, 1 )){
                    ui->textBrowser->append("writeLocalDict 0x1006 fail !");
                }
            }

            m_CANopenStart = false;

        }
    }
    //![3.3] For modbus tcp mode
    else{

    }
}

/*!
 * \brief Monitor state change (open or close)
 * \param state 0:unconnected
 */
void FanMotorUi::monitor_stateChanged(int state)
{
    //![1] If state not change, do nothing
    if(m_monitorState == (bool)state)
        return;

    //![2] Update state to variable "m_monitorState"
    m_monitorState = state;//Update monitor state

    //![3.1] If state > 0 , start  monitor, open timer
    if(state){
        modbusRtuDevice->setTimeout(g_responseTimeout);//!< Set response timeout
        modbusRtuDevice->setNumberOfRetries(g_numberOfTry);//!< Set retry number
        QTimer::singleShot(0, [this]() { monitorTimer_update(); });
    }
    //![3.2] Else, stop monitor , restore the modbus settings
    else{
        modbusRtuDevice->setTimeout(100);//!< Set response timeout
        modbusRtuDevice->setNumberOfRetries(2);//!< Set retry number
    }
}

/*!
 * \brief FanMotorUi::monitorSigleStateChange
 * \param state
 */
void FanMotorUi::monitorSigleStateChange(int state)
{
    int motorAddress = ui->spinBox_InitAdress->value();

    qDebug()<< motorAddress <<"isSigleMotorMonitor:"<<state;

    if(state){//!< Open state

        //! Load "m_specialMotorAdd" with current motor address
        m_specialMotorAdd = motorAddress;

        //! If already start , do nothing
        if(m_monitorState == 1){

        }
        //! If not start yet, now start
        else{
            modbusRtuDevice->setTimeout(g_responseTimeout);//! Set response timeout
            modbusRtuDevice->setNumberOfRetries(g_numberOfTry);//! Set retry number
            QTimer::singleShot(0, [this]() { monitorTimer_update(); });
        }
    }
    else//!< Close state
        m_specialMotorAdd = 0;//!< Load "m_specialMotorAdd" with 0, because no motor have address 0

}

/*!
 * \brief Monitor group and all timer update
 */
void FanMotorUi::monitorTimer_update()
{
    //![1] Wait until no timerout response
    if(m_monitorTimeroutCount > 0){

        //! Delay some time , in case one cycle not complete yet
        QTimer::singleShot(m_monitorTimeroutCount*g_responseTimeout*g_numberOfTry + 50, [this]() { monitorTimer_update(); });
        m_monitorTimeroutCount = 0;
        return;
    }

    //![2] Get Groups pointer
    if(!m_groups)
        m_groups = homewindow::getInstance()->groups();

    //![3] Init variable
    m_monitorTimerPeriod = 0; //!< For count next monitor period time
    m_monitorTimeroutCount = 0; //!< For add next monitor period time when timeout happen
    int startAddress = g_mRealTimeRegisterAddress; //!< Register start address
    int numberOfEntries = g_mRealTimeRegisterCount; //!< 16bit register number
    //![3.1] Pack data unit
    QModbusDataUnit modbusDataUnit(QModbusDataUnit::HoldingRegisters, startAddress, numberOfEntries);

    //![4] For each motor, check state and send request
    foreach(FanGroupInfo *_group, *m_groups){
        foreach(QMotor *_motor, _group->m_motors){

            //![4.1] If is the special motor, we need more information, change the number of entries
            if(_motor->m_address == m_specialMotorAdd){

                modbusDataUnit.setValueCount(g_mRealTimeRegisterMoreCount);//!< Change the number of entries
                //! Send reqeust to special motor
                if (auto *reply = modbusRtuDevice->sendReadRequest(modbusDataUnit, m_specialMotorAdd)) {
                    if (!reply->isFinished()){
                        connect(reply, &QModbusReply::finished, this, &FanMotorUi::monitorReadReady);
                    }
                    else
                        delete reply; //!< Broadcast replies return immediately
                } else {
                    ui->textBrowser->append(tr("motor%1: Read error: ").arg(_motor->m_address) + modbusRtuDevice->errorString());
                }
                m_monitorTimerPeriod += g_responseTimeout;//!< One motor request ,then period value add
            }
            //![4.2] If not the special motor and monitoring has begun, check motor monitor state
            else if(m_monitorState && _motor->isMonitor){

                modbusDataUnit.setValueCount(g_mRealTimeRegisterCount);//!< Change the number of entries
                //! Send reqeust to the not special motor
                if (auto *reply = modbusRtuDevice->sendReadRequest(modbusDataUnit, _motor->m_address)) {
                    if (!reply->isFinished()){
                        connect(reply, &QModbusReply::finished, this, &FanMotorUi::monitorReadReady);
                    }
                    else
                        delete reply; //!< Broadcast replies return immediately
                } else {
                    ui->textBrowser->append(tr("motor%1: Read error: ").arg(_motor->m_address) + modbusRtuDevice->errorString());
                }

                m_monitorTimerPeriod += g_responseTimeout;//!< One motor request ,then period value add

                //! For special motor , we need real time data, so we send reqeust after each other motor reqeust
                if(m_specialMotorAdd > 0){

                    modbusDataUnit.setValueCount(g_mRealTimeRegisterMoreCount);//!< Change the number of entries
                    //! Send reqeust to special motor
                    if (auto *reply = modbusRtuDevice->sendReadRequest(modbusDataUnit, m_specialMotorAdd)) {
                        if (!reply->isFinished()){
                            connect(reply, &QModbusReply::finished, this, &FanMotorUi::monitorReadReady);
                        }
                        else
                            delete reply; //!< Broadcast replies return immediately
                    } else {
                        ui->textBrowser->append(tr("motor%1: Read error: ").arg(_motor->m_address) + modbusRtuDevice->errorString());
                    }
                    m_monitorTimerPeriod += g_responseTimeout;//!< One motor request ,then period value add
                }
            }
        }
    }

    //![5] IF not interrupted ,go on next period
    if(m_monitorState || (m_specialMotorAdd != 0)){
        QTimer::singleShot(m_monitorTimerPeriod*g_numberOfTry, [this]() { monitorTimer_update(); });
    }

}

/*!
 * \brief Monitor group and all read ready
 */
void FanMotorUi::monitorReadReady()
{
    QModbusReply *reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;
    int _motorAddress = reply->serverAddress() ;

    if(!m_groups)
        m_groups = homewindow::getInstance()->groups();


    QMotor *motor = Q_NULLPTR;
    foreach(FanGroupInfo *_group, *m_groups){//!< Get corresponding motor
        //! Judge belong to which group
        if(_motorAddress >= _group->m_startAddress && \
                _motorAddress < _group->m_startAddress + _group->m_motors.count())
        {
            foreach(QMotor *_motor, _group->m_motors){
                if(_motor->m_address == _motorAddress){
                    motor = _motor;
                    break;
                }
            }
        }
    }
    if(!motor)
        return;

    const QModbusDataUnit unit = reply->result();

    if (reply->error() == QModbusDevice::NoError) {

        quint16 *buff = Q_NULLPTR;
        if(unit.startAddress() == g_mRealTimeRegisterAddress){
            buff = (quint16 *)&motor->m_motorController;
            buff += 2;
        }

        if(!buff)
            return;

        for (uint i = 0; i < unit.valueCount(); i++) {
            *buff++ = unit.value(i);
        }

        motor->m_communicationState =FanCommunicationState::m_connect;

    } else if (reply->error() == QModbusDevice::ProtocolError) {

        ui->textBrowser->append(tr("motor%1:Read response error: %2 (Mobus exception: 0x%3)")
                                .arg(motor->m_address).arg(reply->errorString())
                                .arg(reply->rawResult().exceptionCode(), -1, 16));

        motor->m_communicationState =FanCommunicationState::m_comError;
    } else {

        ui->textBrowser->append(tr("motor%1:Read response error: %2 (code: 0x%3)")
                                .arg(motor->m_address).arg(reply->errorString())
                                .arg(reply->error(), -1, 16));

        motor->m_communicationState =FanCommunicationState::m_disconnect;
        m_monitorTimeroutCount++;

    }

    motor->update();//!< Update motor ui

    //! For update sigle motor ui
    if(motor->m_address == m_specialMotorAdd){
            emit updateSigleMotor(1);
    }

    if(m_communication == CommunicationMode::Modbus){
        emit writeMotorRegister(_motorAddress, unit.startAddress(), unit.valueCount());
    }

    reply->deleteLater();

}

/* Group and all motor monitor fuction part end ************************************/


/* Initialize fan / group / all motor fuction part start ******************************/

/*!
 * \brief FanMotorUi::on_clearButton_clicked  slot fuction
 */
void FanMotorUi::on_clearButton_clicked()
{
    ui->textBrowser->clear();
}

/*!
 * \brief FanMotorUi::on_initializeButton_clicked  slot fuction
 */
void FanMotorUi::on_initializeButton_clicked()
{
    if(m_communication == CommunicationMode::Modbus){
        if (!modbusRtuDevice)
            return;

        int startAddress = g_mControllerRegisterAddress; //!< register address
        int numberOfEntries = g_mRatedRegisterCount; //!< 16bit register number
        qDebug() <<"Read number Of entries:"<<numberOfEntries;
        QModbusDataUnit _modbusDataUnit(QModbusDataUnit::HoldingRegisters, startAddress, numberOfEntries);

        if (auto *reply = modbusRtuDevice->sendReadRequest(_modbusDataUnit, ui->spinBox_InitAdress->value())) {
            if (!reply->isFinished()){
                connect(reply, &QModbusReply::finished, this, &FanMotorUi::readInitFGAReady);
            }
            else
                delete reply; //!< Broadcast replies return immediately
        } else {
            ui->textBrowser->append(tr("Read error: ") + modbusRtuDevice->errorString());
        }
    }

    if(!m_currentGroup->m_monitorDialog){
            m_currentGroup->m_monitorDialog = new GroupMonitorDialog(&m_currentGroup->m_motors);
    }
    if(m_currentGroup->m_monitorDialog->isHidden() && SigleMotorDialog::getS_Instance()->isHidden())
        m_currentGroup->m_monitorDialog->show(m_currentGroup->m_groupID, m_motors);
}

/*!
 * \brief FanMotorUi::on_initializeGButton_clicked
 */
void FanMotorUi::on_initializeGButton_clicked()
{
    foreach(QMotor *_motor, m_currentGroup->m_motors){
        if (!modbusRtuDevice)
            return;

        //! Pack data unit
        int startAddress = g_mControllerRegisterAddress; //!< register address
        int numberOfEntries = g_mRatedRegisterCount; //!< 16bit register number
        QModbusDataUnit _modbusDataUnit(QModbusDataUnit::HoldingRegisters, startAddress, numberOfEntries);

        //! Send read request to one motor
        if (auto *reply = modbusRtuDevice->sendReadRequest(_modbusDataUnit, _motor->m_address)) {
            if (!reply->isFinished()){
                connect(reply, &QModbusReply::finished, this, &FanMotorUi::readInitFGAReady);
            }
            else
                delete reply; //!< Broadcast replies return immediately
        } else {
            ui->textBrowser->append(tr("motor%1: Read error: ").arg(_motor->m_address) + modbusRtuDevice->errorString());
        }
    }

    if(!m_currentGroup->m_monitorDialog){
            m_currentGroup->m_monitorDialog = new GroupMonitorDialog(&m_currentGroup->m_motors);
    }
    if(m_currentGroup->m_monitorDialog->isHidden())
        m_currentGroup->m_monitorDialog->show(m_currentGroup->m_groupID, m_motors);
}

/*!
 * \brief FanMotorUi::on_initializeAButton_clicked
 */
void FanMotorUi::on_initializeAButton_clicked()
{
    //![1] Get Groups pointer
    if(!m_groups)
        m_groups = homewindow::getInstance()->groups();

    foreach(FanGroupInfo *_group, *m_groups){

        foreach(QMotor *_motor, _group->m_motors){
            if (!modbusRtuDevice)
                return;

            //Pack data unit
            int startAddress = g_mControllerRegisterAddress; //register address
            int numberOfEntries = g_mRatedRegisterCount; //16bit register number
            QModbusDataUnit _modbusDataUnit(QModbusDataUnit::HoldingRegisters, startAddress, numberOfEntries);

            //Send read request to one motor
            if (auto *reply = modbusRtuDevice->sendReadRequest(_modbusDataUnit, _motor->m_address)) {
                if (!reply->isFinished()){
                    connect(reply, &QModbusReply::finished, this, &FanMotorUi::readInitFGAReady);
                }
                else
                    delete reply; // Broadcast replies return immediately
            } else {
                ui->textBrowser->append(tr("motor%1: Read error: ").arg(_motor->m_address) + modbusRtuDevice->errorString());
            }
        }


    }

    if(!m_currentGroup->m_monitorDialog){
            m_currentGroup->m_monitorDialog = new GroupMonitorDialog(&m_currentGroup->m_motors);
    }
    if(m_currentGroup->m_monitorDialog->isHidden())
        m_currentGroup->m_monitorDialog->show(m_currentGroup->m_groupID, m_motors);
}

/*!
 * \brief Read fan Init parameter read ready
 */
void FanMotorUi::readInitFGAReady()
{
    QModbusReply *reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;

    int _motorAddress = reply->serverAddress() ;

    if(!m_groups)
        m_groups = homewindow::getInstance()->groups();

    QMotor *motor = Q_NULLPTR;
    foreach(FanGroupInfo *_group, *m_groups){//!< Get corresponding motor
        //! Judge belong to which group
        if(_motorAddress >= _group->m_startAddress && \
                _motorAddress < _group->m_startAddress + _group->m_motors.count())
        {
            foreach(QMotor *_motor, _group->m_motors){
                if(_motor->m_address == _motorAddress){
                    motor = _motor;
                    break;
                }
            }
        }
    }

    if(!motor)
        return;

    const QModbusDataUnit unit = reply->result();

    if (reply->error() == QModbusDevice::NoError) {

        quint16 *buff = Q_NULLPTR;
        if(unit.startAddress() == g_mControllerRegisterAddress){
            buff = (quint16 *)&motor->m_motorController;
        }

        if(!buff)
            return;

        for (uint i = 0; i < unit.valueCount(); i++) {
            *buff++ = unit.value(i);
        }

        motor->m_communicationState =FanCommunicationState::m_connect;

    } else if (reply->error() == QModbusDevice::ProtocolError) {

        ui->textBrowser->append(tr("motor%1:Read response error: %2 (Mobus exception: 0x%3)")
                                .arg(motor->m_address).arg(reply->errorString())
                                .arg(reply->rawResult().exceptionCode(), -1, 16));

        motor->m_communicationState =FanCommunicationState::m_comError;
    } else {

        ui->textBrowser->append(tr("motor%1:Read response error: %2 (code: 0x%3)")
                                .arg(motor->m_address).arg(reply->errorString())
                                .arg(reply->error(), -1, 16));

        motor->m_communicationState =FanCommunicationState::m_disconnect;

    }

    motor->update();//!< Update motor ui

    //! For update sigle motor ui
    if(motor->m_address == ui->spinBox_InitAdress->value()){
        if(!SigleMotorDialog::getS_Instance()->isHidden())
            emit updateSigleMotor(2);
    }

    if(m_communication == CommunicationMode::Modbus){
        emit writeMotorRegister(_motorAddress, unit.startAddress(), unit.valueCount());
    }
    reply->deleteLater();
}

/* Initialize fan / group / all motor fuction part end *********************************/

/*!
 * \brief Run or stop fan / group / all
 */
void FanMotorUi::onRunStateButtonClicked()
{
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    if(btn == ui->pushButton_startMotor || btn == ui->pushButton_stopMotor){

        if(m_communication == CommunicationMode::Modbus){
            if (!modbusRtuDevice)
                return;

            //![1] Init data
            int _address = ui->spinBox_InitAdress->value();//!< Server address
            int _pos = ui->spinBox_InitAdress->value()-ui->spinBox_InitAdress->minimum();//!< Position at container
            if(btn == ui->pushButton_startMotor)
                m_motors->at(_pos)->m_motorController.m_runState = FanMotorState::m_run;
            else
                m_motors->at(_pos)->m_motorController.m_runState = FanMotorState::m_stop;
            quint16 *buff = (quint16 *)&m_motors->at(_pos)->m_motorController;
            buff += 2;

            //![2] Pack data to a QModbusDataUnit
            const QModbusDataUnit::RegisterType table = QModbusDataUnit::HoldingRegisters;
            int startAddress = g_mRealTimeRegisterAddress;
            int numberOfEntries = g_mRealTimeRegisterStateCount;
            QModbusDataUnit writeUnit = QModbusDataUnit(table, startAddress, numberOfEntries);
            for (uint i = 0; i < writeUnit.valueCount(); i++) {

                writeUnit.setValue(i, *buff++);
            }
            qDebug() <<writeUnit.valueCount();

            //![3] Send write request to node(adress:)
            if (auto *reply = modbusRtuDevice->sendWriteRequest(writeUnit, _address)) {
                if (!reply->isFinished()) {
                    connect(reply, &QModbusReply::finished, this, [this, reply]() {
                        if (reply->error() == QModbusDevice::ProtocolError) {
                            ui->textBrowser->append(tr("Write response error: %1 (Mobus exception: 0x%2)")
                                                    .arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 16)
                                                    );
                        } else if (reply->error() != QModbusDevice::NoError) {
                            ui->textBrowser->append(tr("Write response error: %1 (code: 0x%2)").
                                                    arg(reply->errorString()).arg(reply->error(), -1, 16));
                        }
                        reply->deleteLater();
                    });
                } else {//!< Broadcast replies return immediately
                    reply->deleteLater();
                }
            } else {
                ui->textBrowser->append(tr("Write error: ") + modbusRtuDevice->errorString());
            }
        }

    }
    else if(btn == ui->pushButton_startMotor_G || btn == ui->pushButton_stopMotor_G){

        if(m_communication == CommunicationMode::Modbus){
            if (!modbusRtuDevice)
                return;

            //![1] Init data
            int _startAddress = ui->spinBox_InitAdress->minimum();//!<Server address
            int _lastAddress = ui->spinBox_InitAdress->maximum();
            int _pos = ui->spinBox_InitAdress->value()-ui->spinBox_InitAdress->minimum();//!<Position at container
            if(btn == ui->pushButton_startMotor_G)
                m_motors->at(_pos)->m_motorController.m_runState = FanMotorState::m_run;
            else
                m_motors->at(_pos)->m_motorController.m_runState = FanMotorState::m_stop;
            quint16 *buff = (quint16 *)&m_motors->at(_pos)->m_motorController;
            buff += 2;

            //![2] Pack data to a QModbusDataUnit
            const QModbusDataUnit::RegisterType table = QModbusDataUnit::HoldingRegisters;
            int startAddress = g_mRealTimeRegisterAddress;
            int numberOfEntries = g_mRealTimeRegisterStateCount;
            QModbusDataUnit writeUnit = QModbusDataUnit(table, startAddress, numberOfEntries);
            for (uint i = 0; i < writeUnit.valueCount(); i++) {

                writeUnit.setValue(i, *buff++);
            }
            qDebug() <<writeUnit.valueCount();

            //![3] Send write request to node(adress:i) at current group
            for(int i = _startAddress; i <= _lastAddress; i++){

                if (auto *reply = modbusRtuDevice->sendWriteRequest(writeUnit, i)) {
                    if (!reply->isFinished()) {
                        connect(reply, &QModbusReply::finished, this, [this, reply]() {
                            if (reply->error() == QModbusDevice::ProtocolError) {
                                ui->textBrowser->append(tr("Write response error: %1 (Mobus exception: 0x%2)")
                                                        .arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 16)
                                                        );
                            } else if (reply->error() != QModbusDevice::NoError) {
                                ui->textBrowser->append(tr("Write response error: %1 (code: 0x%2)").
                                                        arg(reply->errorString()).arg(reply->error(), -1, 16));
                            }
                            reply->deleteLater();
                        });
                    } else {//!< Broadcast replies return immediately
                        reply->deleteLater();
                    }
                } else {
                    ui->textBrowser->append(tr("Write error: ") + modbusRtuDevice->errorString());
                }
            }
        }

    }
    else if(btn == ui->pushButton_startMotor_A || btn == ui->pushButton_stopMotor_A){
        if(m_communication == CommunicationMode::Modbus){
            if (!modbusRtuDevice)
                return;

            //![1] Init data
            int _address = 0;//!Broadcast address
            int _pos = ui->spinBox_InitAdress->value()-ui->spinBox_InitAdress->minimum();//!< Position at container
            if(btn == ui->pushButton_startMotor_A)
                m_motors->at(_pos)->m_motorController.m_runState = FanMotorState::m_run;
            else
                m_motors->at(_pos)->m_motorController.m_runState = FanMotorState::m_stop;
            quint16 *buff = (quint16 *)&m_motors->at(_pos)->m_motorController;
            buff += 2;

            //![2] Pack data to a QModbusDataUnit
            const QModbusDataUnit::RegisterType table = QModbusDataUnit::HoldingRegisters;
            int startAddress = g_mRealTimeRegisterAddress;
            int numberOfEntries = g_mRealTimeRegisterStateCount;
            QModbusDataUnit writeUnit = QModbusDataUnit(table, startAddress, numberOfEntries);
            for (uint i = 0; i < writeUnit.valueCount(); i++) {

                writeUnit.setValue(i, *buff++);
            }
            qDebug() <<writeUnit.valueCount();

            //![3] Send write request to node(adress:0) : Broadcast
            if (auto *reply = modbusRtuDevice->sendWriteRequest(writeUnit, _address)) {
                if (!reply->isFinished()) {
                } else {//!< Broadcast replies return immediately
                    reply->deleteLater();
                }
            } else {
                ui->textBrowser->append(tr("Write error: ") + modbusRtuDevice->errorString());
            }
        }

    }
}

/*!
 * \brief FanMotorUi::onMotorStartButtonClicked
 *        For SigleMotorDialog ui
 */
void FanMotorUi::onMotorStartButtonClicked()
{
    ui->pushButton_startMotor->clicked();
}

/*!
 * \brief FanMotorUi::onMotorStopButtonClicked
 *        For SigleMotorDialog ui
 */
void FanMotorUi::onMotorStopButtonClicked()
{
    ui->pushButton_stopMotor->clicked();
}

/*!
 * \brief FanMotorUi::onFanButtonclicked
 *        Show SigleMotorDialog ui
 */
void FanMotorUi::onFanButtonclicked()
{
    QPushButton* btn = qobject_cast<QPushButton*>(sender());

    if(!m_motors)
        return;

    for(int i = 0; i < m_motorButtons.count(); i++){
        if(btn == m_motorButtons.at(i)){

            ui->spinBox_InitAdress->setValue(m_motors->at(i)->m_address);
            SigleMotorDialog::getS_Instance()->show(m_motors->at(i));

            if(m_communication == CommunicationMode::Modbus ){
                if(!modbusRtuDevice)
                    emit updateSigleMotorState(0);
                else{
                    bool connected = (modbusRtuDevice->state() != QModbusDevice::UnconnectedState);
                    emit updateSigleMotorState(connected);
                }
            }
        }

    }
}

/*!
 * \brief FanMotorUi::onSigleMotorInitSetClicked
 * \param table
 * \param data
 */
void FanMotorUi::onSigleMotorInitSetClicked(QTableWidget *table, quint16 data)
{
    QAbstractItemModel *__model = ui->table_settings->model();
    QAbstractItemModel *__srcmodel = table->model();
    double _data;
    for(unsigned char i = 0; i<3; i++){
        for(unsigned char j = 1; j<5; j++){
            _data = __srcmodel->data(__model->index(i, j)).toDouble();
            __model->setData(__model->index(i,j), _data);
        }
    }

    int _fanType = (data >> 12) & 0x000f;
    ui->comboBox_fanType->setCurrentIndex(_fanType);
    int _fanPressure = (data >> 10) & 0x0003;
    ui->comboBox_fanPressure->setCurrentIndex(_fanPressure);
    int _fanreGulation = (data >> 8) & 0x0003;
    ui->comboBox_fanreGulation->setCurrentIndex(_fanreGulation);
    int _fanControlMode = (data >> 7) & 0x0001;
    ui->comboBox_fanControlMode->setCurrentIndex(_fanControlMode);

    on_pushButton_InitSetF_clicked();
}

/*!
 * \brief FanMotorUi::onSetPI
 */
void FanMotorUi::onSetPI()
{
    if(m_communication == CommunicationMode::Modbus){
        if (!modbusRtuDevice)
            return;

        //![1] Init data

        int _address = ui->spinBox_InitAdress->value();//Server address
        int _pos = ui->spinBox_InitAdress->value()-ui->spinBox_InitAdress->minimum();//Position at container
        quint16 *buff = (quint16 *)&m_motors->at(_pos)->m_PIPara;

        //![2] Pack data to a QModbusDataUnit
        const QModbusDataUnit::RegisterType table = QModbusDataUnit::HoldingRegisters;
        int startAddress = g_mPIParaRegisterAddress;
        int numberOfEntries = g_mPIParaRegisterMoreCount;
        QModbusDataUnit writeUnit = QModbusDataUnit(table, startAddress, numberOfEntries);
        for (uint i = 0; i < writeUnit.valueCount(); i++) {

            writeUnit.setValue(i, *buff++);
        }
        qDebug() <<writeUnit.valueCount();

        //![3] Send write request to node(adress:)
        if (auto *reply = modbusRtuDevice->sendWriteRequest(writeUnit, _address)) {
            if (!reply->isFinished()) {
                connect(reply, &QModbusReply::finished, this, [this, reply]() {
                    if (reply->error() == QModbusDevice::ProtocolError) {
                        ui->textBrowser->append(tr("Write response error: %1 (Mobus exception: 0x%2)")
                                                .arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 16)
                                                );
                    } else if (reply->error() != QModbusDevice::NoError) {
                        ui->textBrowser->append(tr("Write response error: %1 (code: 0x%2)").
                                                arg(reply->errorString()).arg(reply->error(), -1, 16));
                    }
                    reply->deleteLater();
                });
            } else {//!< Broadcast replies return immediately
                reply->deleteLater();
            }
        } else {
            ui->textBrowser->append(tr("Write error: ") + modbusRtuDevice->errorString());
        }
    }
}

/*!
 * \brief FanMotorUi::onReadPI
 */
void FanMotorUi::onReadPI()
{
    if(m_communication == CommunicationMode::Modbus){
        if (!modbusRtuDevice)
            return;

        int startAddress = g_mPIParaRegisterAddress; //!< register address
        int numberOfEntries = g_mPIParaRegisterMoreCount; //!< 16bit register number
        qDebug() <<"Read number Of entries:"<<numberOfEntries;
        QModbusDataUnit _modbusDataUnit(QModbusDataUnit::HoldingRegisters, startAddress, numberOfEntries);

        if (auto *reply = modbusRtuDevice->sendReadRequest(_modbusDataUnit, ui->spinBox_InitAdress->value())) {
            if (!reply->isFinished()){
                connect(reply, &QModbusReply::finished, this, &FanMotorUi::readPIReady);
            }
            else
                delete reply; //!< Broadcast replies return immediately
        } else {
            ui->textBrowser->append(tr("Read error: ") + modbusRtuDevice->errorString());
        }
    }
}

/*!
 * \brief FanMotorUi::readPIReady
 *        Read fan Init parameter Ready
 */
void FanMotorUi::readPIReady()
{
    QModbusReply *reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;

    int __pos = ui->spinBox_InitAdress->value() - ui->spinBox_InitAdress->minimum();//!< Current motor pos of container

    const QModbusDataUnit unit = reply->result();//!< Get reply data unit

    //! Reply no error
    if (reply->error() == QModbusDevice::NoError) {

        quint16 *buff;
        if(unit.startAddress() == g_mPIParaRegisterAddress){//!< Make sure is Init data
            buff = (quint16 *)&m_motors->at(__pos)->m_PIPara;//!< Get current motor's initSetttings pointer
        }

        //! Put data to current motor's initSetttings
        for (uint i = 0; i < unit.valueCount(); i++) {

            *buff++ = unit.value(i);

            const QString entry = tr("Address: %1, Value: %2").arg(QString::number(unit.startAddress()+i, 16))
                    .arg(QString::number(unit.value(i), 16));
            ui->textBrowser->append(entry);//!< Show out the data
        }

        m_motors->at(__pos)->m_communicationState =FanCommunicationState::m_connect;//!< Update communication state

    }
    //! Reply protocol error
    else if (reply->error() == QModbusDevice::ProtocolError) {
        ui->textBrowser->append(tr("Read response error: %1 (Mobus exception: 0x%2)").
                                arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 16));
        m_motors->at(__pos)->m_communicationState =FanCommunicationState::m_comError;//!< Update communication state
    }
    //! Reply timeout
    else {
        ui->textBrowser->append(tr("Read response error: %1 (code: 0x%2)").
                                arg(reply->errorString()).arg(reply->error(), -1, 16));
        m_motors->at(__pos)->m_communicationState =FanCommunicationState::m_disconnect;//!< Update communication state
    }

    m_motors->at(__pos)->update();//!< Update motor data to ui

    //! For update sigle motor ui
    if(m_motors->at(__pos)->m_address == ui->spinBox_InitAdress->value()){
        if(!SigleMotorDialog::getS_Instance()->isHidden())
            emit updateSigleMotor(3);
    }

    if(m_communication == CommunicationMode::Modbus){
        emit writeMotorRegister(m_motors->at(__pos)->m_address, unit.startAddress(), unit.valueCount());
    }

    reply->deleteLater();
}



#define CHECK_SIZE_EQUALS(req) \
    do { \
        if (req.dataSize() != QModbusRequest::minimumDataSize(req)) { \
            qDebug() << "(Server) The request's data size does not equal the expected size."; \
            return QModbusExceptionResponse(req.functionCode(), \
                                            QModbusExceptionResponse::IllegalDataValue); \
        } \
    } while (0)

#define CHECK_SIZE_LESS_THAN(req) \
    do { \
        if (req.dataSize() < QModbusRequest::minimumDataSize(req)) { \
            qDebug() << "(Server) The request's data size is less than the expected size."; \
            return QModbusExceptionResponse(req.functionCode(), \
                                            QModbusExceptionResponse::IllegalDataValue); \
        } \
    } while (0)

QModbusResponse FanMotorUi::processReadHoldingRegistersRequest(const QModbusRequest &rqst)
{
    return readBytes(rqst, QModbusDataUnit::HoldingRegisters);
}
QModbusResponse FanMotorUi::readBytes(const QModbusPdu &request,
                                                QModbusDataUnit::RegisterType unitType)
{
    CHECK_SIZE_EQUALS(request);
    quint16 address, count;
    request.decodeData(&address, &count);

    if ((count < 0x0001) || (count > 0x007D)) {
        return QModbusExceptionResponse(request.functionCode(),
            QModbusExceptionResponse::IllegalDataValue);
    }

    // Get the requested range out of the registers.
    QModbusDataUnit unit(unitType, address, count);

    //! address :
    //! bit 16:31 motor address
    //! bit 0:15 register address

    quint16 motorAdd = (address >> 8) & 0x00ff ;
    quint16 registerAdd = (address) & 0x00ff ;

    QMotor *motor;
    foreach(FanGroupInfo *_group, *m_groups){//!< Get corresponding motor
        //! Judge belong to which group
        if(motorAdd >= _group->m_startAddress && \
                motorAdd < _group->m_startAddress + _group->m_motors.count())
        {
            foreach(QMotor *_motor, _group->m_motors){
                if(_motor->m_address == motorAdd){
                    motor = _motor;
                    break;
                }
            }
        }
    }
    if(!motor){
        return QModbusExceptionResponse(request.functionCode(),
            QModbusExceptionResponse::IllegalDataValue);
    }

    quint16 * buff;
    if(registerAdd == g_mRealTimeRegisterAddress && count <= g_mRealTimeRegisterMoreCount)
        buff = (quint16 *)&motor->m_motorController;

    if(registerAdd == g_mSettingsRegisterAddress)
        buff = (quint16 *)&motor->m_initSetttings;

    for (uint i = 0; i < unit.valueCount(); i++) {

        unit.setValue(i, *buff++);
    }

    return QModbusResponse(request.functionCode(), quint8(count * 2), unit.values());
}

QModbusResponse FanMotorUi::processWriteMultipleRegistersRequest(
    const QModbusRequest &request)
{
    CHECK_SIZE_LESS_THAN(request);
    quint16 address, numberOfRegisters;
    quint8 byteCount;
    request.decodeData(&address, &numberOfRegisters, &byteCount);

    //! byte count does not match number of data bytes following or register count
    if ((byteCount != (request.dataSize() - 5 )) || (byteCount != (numberOfRegisters * 2))) {
        return QModbusExceptionResponse(request.functionCode(),
                                        QModbusExceptionResponse::IllegalDataValue);
    }

    if ((numberOfRegisters < 0x0001) || (numberOfRegisters > 0x007B)) {
        return QModbusExceptionResponse(request.functionCode(),
            QModbusExceptionResponse::IllegalDataValue);
    }

    quint16 motorAdd = (address >> 8) & 0x00ff ;
    quint16 registerAdd = (address) & 0x00ff ;

    if(motorAdd != 0){

        QMotor *motor;
        foreach(FanGroupInfo *_group, *m_groups){//!< Get corresponding motor
            //! Judge belong to which group
            if(motorAdd >= _group->m_startAddress && \
                    motorAdd < _group->m_startAddress + _group->m_motors.count())
            {
                foreach(QMotor *_motor, _group->m_motors){
                    if(_motor->m_address == motorAdd){
                        motor = _motor;
                        break;
                    }
                }
            }
        }

        if(!motor){
            return QModbusExceptionResponse(request.functionCode(),
                                            QModbusExceptionResponse::IllegalDataValue);
        }

        quint16 * buff;
        if(registerAdd == g_mRealTimeRegisterAddress)
            buff = (quint16 *)&motor->m_motorController;
        if(registerAdd == g_mSettingsRegisterAddress)
            buff = (quint16 *)&motor->m_initSetttings;
        else{
            return QModbusExceptionResponse(request.functionCode(),
                                            QModbusExceptionResponse::IllegalDataValue);
        }
        const QByteArray pduData = request.data().remove(0,5);
        QDataStream stream(pduData);
        quint16 tmp;
        for (int i = 0; i < numberOfRegisters; i++) {
            stream >> tmp;
            *buff++ = tmp;
        }

        //! Update ui
        motor->update();

        //! Update table
        if(registerAdd == g_mSettingsRegisterAddress){
            //Update data to settings table
            QAbstractItemModel *__model = ui->table_settings->model();
            quint16 *__buffPtr = (quint16 *)&motor->m_initSetttings;
            for(unsigned char i = 0; i<3; i++){
                for(unsigned char j = 1; j<5; j++){
                    __model->setData(__model->index(i,j), (double)(*__buffPtr) * 0.01);
                    __buffPtr++;
                }
            }

            int _fanType = (*__buffPtr >> 12) & 0x000f;
            ui->comboBox_fanType->setCurrentIndex(_fanType);
            int _fanPressure = (*__buffPtr >> 10) & 0x0003;
            ui->comboBox_fanPressure->setCurrentIndex(_fanPressure);
            int _fanreGulation = (*__buffPtr >> 8) & 0x0003;
            ui->comboBox_fanreGulation->setCurrentIndex(_fanreGulation);
            int _fanControlMode = (*__buffPtr >> 7) & 0x0001;
            ui->comboBox_fanControlMode->setCurrentIndex(_fanControlMode);
        }
    }
    else{
        quint16 * buff;
        const QByteArray pduData = request.data().remove(0,5);
        foreach(FanGroupInfo *group, *m_groups){
            foreach(QMotor *motor, group->m_motors){
                if(registerAdd == g_mRealTimeRegisterAddress)
                    buff = (quint16 *)&motor->m_motorController;
                if(registerAdd == g_mSettingsRegisterAddress)
                    buff = (quint16 *)&motor->m_initSetttings;
                else{
                    return QModbusExceptionResponse(request.functionCode(),
                                                    QModbusExceptionResponse::IllegalDataValue);
                }

                QDataStream stream(pduData);
                quint16 tmp;
                for (int i = 0; i < numberOfRegisters; i++) {
                    stream >> tmp;
                    *buff++ = tmp;
                }
            }
        }
    }

    return QModbusResponse(request.functionCode(), address, numberOfRegisters);
}

QModbusResponse FanMotorUi::processWriteSingleRegisterRequest(const QModbusRequest &rqst)
{
    return writeSingle(rqst, QModbusDataUnit::HoldingRegisters);
}

QModbusResponse FanMotorUi::writeSingle(const QModbusPdu &request,
                                                  QModbusDataUnit::RegisterType unitType)
{
    Q_UNUSED(unitType)
    CHECK_SIZE_EQUALS(request);
    quint16 address, value;
    request.decodeData(&address, &value);

    quint16 motorAdd = (address >> 8) & 0x00ff ;
    quint16 registerAdd = (address) & 0x00ff ;

    QMotor *motor;
    foreach(FanGroupInfo *_group, *m_groups){//!< Get corresponding motor
        //! Judge belong to which group
        if(motorAdd >= _group->m_startAddress && \
                motorAdd < _group->m_startAddress + _group->m_motors.count())
        {
            foreach(QMotor *_motor, _group->m_motors){
                if(_motor->m_address == motorAdd){
                    motor = _motor;
                    break;
                }
            }
        }
    }
    if(!motor){
        return QModbusExceptionResponse(request.functionCode(),
            QModbusExceptionResponse::IllegalDataValue);
    }

    quint16 * buff;
    if(registerAdd == g_mRealTimeRegisterAddress)
        buff = (quint16 *)&motor->m_motorController;

    *buff = value;

    return QModbusResponse(request.functionCode(), address, value);
}

QModbusResponse FanMotorUi::processRequest(const QModbusPdu &request)
{
    switch (request.functionCode()) {

    case QModbusRequest::ReadHoldingRegisters:
        return processReadHoldingRegistersRequest(request);

    case QModbusRequest::WriteSingleRegister:
        return processWriteSingleRegisterRequest(request);

    case QModbusRequest::WriteMultipleRegisters:
        return processWriteMultipleRegistersRequest(request);

    default:
        break;
    }

    return QModbusExceptionResponse(request.functionCode(),
        QModbusExceptionResponse::IllegalFunction);
}


HModbusTcpServer::HModbusTcpServer(QObject *parent)
{
    Q_UNUSED(parent)

}

QModbusResponse HModbusTcpServer::processRequest(const QModbusPdu &request)
{
    return FanMotorUi::getS_Instance()->processRequest(request);
}


void FanMotorUi::sendCommand(FCommandRegister fcr)
{
    if(!modbusTcpDevice && modbusTcpDevice->state() != QModbusDevice::ConnectedState)
        return;

    int startAddress = g_motorCommandAddress; //!< Register start address
    int numberOfEntries = g_readCommandAddress; //!< 16bit register number
    //! Pack data unit
    QModbusDataUnit writeUnit(QModbusDataUnit::HoldingRegisters, startAddress, numberOfEntries);

    quint16 *buff = (quint16 *) &fcr;
    for (uint i = 0; i < writeUnit.valueCount(); i++) {

        writeUnit.setValue(i, *buff++);
    }

    quint16 address = 1;
    //! Send write request to node(adress:)
    if (auto *reply = modbusTcpDevice->sendWriteRequest(writeUnit, address)) {
        if (!reply->isFinished()) {
            connect(reply, &QModbusReply::finished, this, [this, reply]() {
                if (reply->error() == QModbusDevice::ProtocolError) {
                    ui->textBrowser->append(tr("Write response error: %1 (Mobus exception: 0x%2)")
                                            .arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 16)
                                            );
                } else if (reply->error() != QModbusDevice::NoError) {
                    ui->textBrowser->append(tr("Write response error: %1 (code: 0x%2)").
                                            arg(reply->errorString()).arg(reply->error(), -1, 16));
                }
                reply->deleteLater();
            });
        } else {//!< Broadcast replies return immediately
            reply->deleteLater();
        }
    } else {
        ui->textBrowser->append(tr("Write error: ") + modbusTcpDevice->errorString());
    }
}
