#include "fanmotorui.h"
#include "ui_fanmotorui.h"

#include <QModbusTcpClient>
#include <QModbusRtuSerialMaster>
#include <QStandardItemModel>
#include <QTimer>
#include <QTableWidget>

#include <QDebug>

#include "lamp/qcw_indicatorlamp.h"
#include"fanmotor/qmotor.h"
#include "userui/mplotui.h"
#include "userui/homewindow.h"
#include "dialogs/settingdialog.h"
#include "mainwindow/mainwindow.h"
#include "canui.h"
#include "dialogs/siglemotordialog.h"

const int g_responseTimeout = 50;
const int g_numberOfTry = 2;

FanMotorUi *FanMotorUi::s_Instance = Q_NULLPTR;

/*CAN comunication fuction*/
void FanMotorUi::heartbeatError(CO_Data *d, unsigned char heartbeatID)
{
    UNS8 __address = heartbeatID - 1;

    m_motors->at(__address)->m_communicationState =FanCommunicationState::m_disconnect;

    m_motors->at(__address)->update();

    qDebug()<<d->nodeState<<"heartbeatError"<<heartbeatID;
}
void master_heartbeatError(CO_Data* d, UNS8 heartbeatID)
{
    FanMotorUi::getS_Instance()->heartbeatError(d, heartbeatID);

}

void master_post_sync(CO_Data* d)
{
    qDebug()<<d->nodeState;
}

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

void master_post_SlaveStateChange(CO_Data* d, UNS8 nodeId, e_nodeState newNodeState)
{
    FanMotorUi::getS_Instance()->post_SlaveStateChange(d,nodeId,newNodeState);

}

UNS32 OnMotoRealtimeDataUpdate(CO_Data* d, const indextable * unsused_indextable, UNS8 unsused_bSubindex)
{
    qDebug()<<d->nodeState<<"OnMotoRealtimeDataUpdate"<<unsused_indextable->bSubCount<<unsused_bSubindex;
    return 0;
}
UNS32 OnMotorParaUpdate(CO_Data* d, const indextable * unsused_indextable, UNS8 unsused_bSubindex)
{
    qDebug()<<d->nodeState<<"OnMotorParaUpdate"<<unsused_indextable->bSubCount<<unsused_bSubindex;
    return 0;
}

void FanMotorUi::messageShow(const QString &s)//From can receive thread
{
    ui->textBrowser->append(s);
}

void FanMotorUi::messageHandle(const QCanBusFrame &frame)//From can receive thread for frame handle
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
/*CAN comunication fuction*/


FanMotorUi::FanMotorUi(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FanMotorUi)
  ,m_groups(Q_NULLPTR)
  ,modbusDevice(Q_NULLPTR)
  ,m_motors(Q_NULLPTR)
  ,m_masterBoard{new s_BOARD{}}
  ,m_canThread{CanThread::getInstance()}
  ,m_currentGroup(Q_NULLPTR)
  ,m_monitorTimer(new QTimer)
{
    ui->setupUi(this);
    qDebug("FanMotorUi init");

    //Init fan data in dictionary and register callback fuction
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
    for(int i = 0; i < 15; i++){//All fan buttons use one slot fuction
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


    m_masterBoard->busname = const_cast<char *>("master");//Init can board information
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

    ui->textBrowser->document ()->setMaximumBlockCount (50);//Set textBrowser max block count

}

void FanMotorUi::on_connectButton_clicked()//open
{
    //Tcp connect
    if(ui->radioButton_tcp->isChecked()){

        onStateChanged(1);
    }

    //CAN connect
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

        m_canThread->mStart(true);//open canport and canopen receive thread

        master_Data.heartbeatError = master_heartbeatError;
        master_Data.post_sync = master_post_sync;

        master_Data.post_SlaveStateChange = master_post_SlaveStateChange;

        for(int i = 1; i <= m_motorNum; i++){//Heartbeat

            {//0x1016 :   Consumer Heartbeat Time  nodeid(bit 23 : 16) + time(bit 15 : 0) ms
                UNS32 _sourceData = 1100 + (i << 16);
                UNS32 _dataSize = sizeof(UNS32);
                if(OD_SUCCESSFUL != writeLocalDict(&master_Data, (UNS16)0x1016, (UNS8)i, (UNS32*)&_sourceData, &_dataSize, 1 )){
                    ui->textBrowser->append(tr("writeLocalDict 0x1016 %1: fail !").arg(i));
                }
            }

        }

        TimerInit();
        StartTimerLoop(&InitNodes);

        _canUi->setEnabled(false);//disable canui

        onStateChanged(1);
    }

    //Modbus connect
    if(ui->radioButton_modbus->isChecked()){
        SerialPortSettings::Settings mSerialPortSettings;
        //[1]Read serialport settings in "/SuperConsole.ini" (update in "SerialPortSettingsDialog" class)
        QSettings settings(QDir::currentPath() + "/SuperConsole.ini", QSettings::IniFormat);
        settings.beginGroup("SerialPortSettingsDialog");
        mSerialPortSettings.name = settings.value("SerialPortname", "").toString();
        mSerialPortSettings.baudRate = static_cast<QSerialPort::BaudRate>(settings.value("SerialPortBaudRate", "9600").toInt());
        mSerialPortSettings.dataBits = static_cast<QSerialPort::DataBits>(settings.value("SerialPortDataBits", "8").toInt());
        mSerialPortSettings.parity = static_cast<QSerialPort::Parity>(settings.value("SerialPortParity", "0").toInt());
        mSerialPortSettings.stopBits = static_cast<QSerialPort::StopBits>(settings.value("SerialPortStopBits", "1").toInt());
        mSerialPortSettings.flowControl = static_cast<QSerialPort::FlowControl>(settings.value("SerialPortFlowControl", "0").toInt());
        settings.endGroup();

        //[2] Make sure modbus device was not created yet
        if (modbusDevice) {
            modbusDevice->disconnectDevice();
            delete modbusDevice;
            modbusDevice = Q_NULLPTR;
        }

        //[3] Create modbus RTU client device and monitor error,state
        modbusDevice = new QModbusRtuSerialMaster(this);
        connect(modbusDevice, &QModbusClient::errorOccurred, [this](QModbusDevice::Error) {
            statusBar()->showMessage(modbusDevice->errorString(), 3000);
        });
        if (!modbusDevice) {
            statusBar()->showMessage(tr("Could not create Modbus client."), 3000);
            return;
        } else {
            connect(modbusDevice, &QModbusClient::stateChanged,
                    this, &FanMotorUi::onStateChanged);//Monitor modbus device state
        }

        statusBar()->clearMessage();//Clear status bar first, we will use it

        //[4] Set serialport parameter and connect device
        if (modbusDevice->state() != QModbusDevice::ConnectedState) {

            modbusDevice->setConnectionParameter(QModbusDevice::SerialPortNameParameter,
                                                 mSerialPortSettings.name);
            modbusDevice->setConnectionParameter(QModbusDevice::SerialBaudRateParameter,
                                                 mSerialPortSettings.baudRate);
            modbusDevice->setConnectionParameter(QModbusDevice::SerialDataBitsParameter,
                                                 mSerialPortSettings.dataBits);
            modbusDevice->setConnectionParameter(QModbusDevice::SerialParityParameter,
                                                 mSerialPortSettings.parity);
            modbusDevice->setConnectionParameter(QModbusDevice::SerialStopBitsParameter,
                                                 mSerialPortSettings.stopBits);

            modbusDevice->setTimeout(100);//Set response timeout 300ms
            modbusDevice->setNumberOfRetries(2);//Set retry number 2


            //Now try connect modbus device
            if (!modbusDevice->connectDevice()) {//Connect failed
                statusBar()->showMessage(tr("Modbus connect failed: ") + modbusDevice->errorString(), 3000);
            } else {//Connect  successfully
            }
        }
    }

}

void FanMotorUi::on_disconnectButton_clicked()//close
{
    //Tcp disconnect
    if(m_communication == CommunicationMode::Tcp){

    //    onStateChanged(0);
    }

    //CAN disconnect
    if(m_communication == CommunicationMode::CANbus){
        if(m_CANopenStart){

        }

        m_canThread->mStop();               //then close thread
        disconnect(m_canThread, SIGNAL(message(QString)),
                this, SLOT(messageShow(QString)));

        disconnect(m_canThread, SIGNAL(message(QCanBusFrame)),
                this, SLOT(messageHandle(QCanBusFrame)));

        StopTimerLoop(&Exit);

        canClose(&master_Data);            //final close canport

        CANUi *_canUi = CANUi::getS_Instance();
        _canUi->setEnabled(true);
        onStateChanged(0);

    }

    //Modbus disconnect
    if(m_communication == CommunicationMode::Modbus){
        if(!modbusDevice)
            return;
        if(modbusDevice->state() == QModbusDevice::ConnectedState){
            modbusDevice->disconnectDevice();
        }
    }

    if(ui->checkBox_monitorASW->isChecked())
        ui->checkBox_monitorASW->setChecked(false);
    if(ui->checkBox_monitorGSW->isChecked())
        ui->checkBox_monitorGSW->setChecked(false);

}

void FanMotorUi::onStateChanged(int state)//Monitor modbus device state
{
    bool connected = (state != 0);

    //Modbus state change
    if(ui->radioButton_modbus->isChecked()){
//        bool connected = (state != QModbusDevice::UnconnectedState);
        if(connected){
            m_communication = CommunicationMode::Modbus;
            statusBar()->showMessage(tr("Modbus connected !"), 3000);
        }
        else{
            m_communication = CommunicationMode::Init;
            statusBar()->showMessage(tr("Modbus disonnected !"), 3000);
        }
    }

    //CAN state change
    if(ui->radioButton_can->isChecked()){

        if(connected){
            m_communication = CommunicationMode::CANbus;
            statusBar()->showMessage(tr("CAN connected !"), 3000);
        }
        else{
            m_communication = CommunicationMode::Init;
            statusBar()->showMessage(tr("CAN disonnected !"), 3000);
        }
    }

    //Tcp state change
    if(ui->radioButton_tcp->isChecked()){

        if(connected){
            m_communication = CommunicationMode::Tcp;
            statusBar()->showMessage(tr("CAN connected !"), 3000);
        }
        else{
            m_communication = CommunicationMode::Init;
            statusBar()->showMessage(tr("CAN disonnected !"), 3000);
        }
    }

    ui->connectButton->setEnabled(!connected);
    ui->disconnectButton->setEnabled(connected);
    ui->groupBox_comState->setEnabled(!connected);

    ui->checkBox_monitorGSW->setEnabled(connected);
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

void FanMotorUi::on_spinBox_motorNum_valueChanged(int arg1)//Change current group motor number
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

    for(int i = arg1; i < m_motorButtons.count(); i++)//For reduce group
    {
        m_motorButtons.at(i)->setEnabled(false);
    }
    for(int i = 0; i < arg1; i++)        //For add group
    {
        m_motorButtons.at(i)->setEnabled(true);
    }

    if(m_currentGroup)//Update max address at current address
        ui->spinBox_InitAdress->setMaximum(m_currentGroup->m_startAddress + m_motorNum - 1);
}

void FanMotorUi::on_settingsButton_clicked()
{
    SettingDialog* settingDialog = new SettingDialog;
    settingDialog->exec();//exec: if we don't close dialog ,we can't operate other window
}

FanMotorUi *FanMotorUi::getS_Instance()
{
    if(!s_Instance)
    {
        s_Instance = new FanMotorUi;
    }
    return s_Instance;
}

void FanMotorUi::deleteInstance()
{
    if(s_Instance)
        s_Instance->deleteLater();
}

FanMotorUi::~FanMotorUi()
{
    if(modbusDevice && modbusDevice->state() == QModbusDevice::ConnectedState){
        modbusDevice->disconnectDevice();
    }
    SigleMotorDialog::deleteInstance();
    qDebug()<<"FanMotorUi exit";
    s_Instance = Q_NULLPTR;
    delete ui;
}

QStatusBar* FanMotorUi::statusBar()//Get mainwindow's status bar
{
    return MainWindow::getInstance()->getStatusBar();
}

void FanMotorUi::changeGroup(FanGroupInfo *group)//Change a group to show
{
    if(m_currentGroup == group)//The same group, do nothing
        return;

    //Get group parameter
    m_currentGroup = group;//Get group pointer
    m_motors = &m_currentGroup->m_motors;//Get motor container pointer
    int _startAddress = m_currentGroup->m_startAddress;//Get current group start address
    int _fanMaxNumber = m_currentGroup->m_fanMaxNumber;//Get current group
    int _fanNumber = m_motors->count();//Get current group motor number

    //If current group motor number is 0, we reset it to maxNumber
    if(m_motors->count() == 0){
        _fanNumber = _fanMaxNumber;
        for(int i = 0; i< _fanNumber; i++){//Init table
            m_motors->push_back(new QMotor(i+_startAddress));
        }
    }

    //Update widget
    ui->lcdNumber_groupNum->display(m_currentGroup->m_groupID);
    ui->spinBox_motorNum->setValue(_fanNumber);//Motor number spinBox set the right number
    ui->spinBox_motorNum->setMaximum(_fanMaxNumber);//Motor number spinBox set maxNumber
    ui->spinBox_InitAdress->setMinimum(_startAddress);//SpinBox_InitAdress for first fan Init set or read
    ui->spinBox_InitAdress->setMaximum(_startAddress + _fanNumber - 1);//Last address

    ui->checkBox_monitorGSW->setChecked(m_currentGroup->isMonitor);//Is monitor

}

/*Init parameter set and read fuction*/
void FanMotorUi::on_pushButton_InitSetF_clicked()//Set one fan Init parameter
{
    if(m_communication == CommunicationMode::Modbus){
        if (!modbusDevice)
            return;

        //[1] Init data
        QAbstractItemModel *__model = ui->table_settings->model();
        int _address = ui->spinBox_InitAdress->value();//Server address
        int _pos = ui->spinBox_InitAdress->value()-ui->spinBox_InitAdress->minimum();//Position at container
        quint16 *buff = (quint16 *)&m_motors->at(_pos)->m_initSetttings;

        //[2] Get data from tableview
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

        //[3] Pack data to a QModbusDataUnit
        buff = (quint16 *)&m_motors->at(_pos)->m_initSetttings;
        const QModbusDataUnit::RegisterType table = QModbusDataUnit::HoldingRegisters;
        int startAddress = g_mSettingsRegisterAddress;
        int numberOfEntries = g_mSettingsRegisterCount;
        QModbusDataUnit writeUnit = QModbusDataUnit(table, startAddress, numberOfEntries);
        for (uint i = 0; i < writeUnit.valueCount(); i++) {

            writeUnit.setValue(i, *buff++);
        }
        qDebug() <<writeUnit.valueCount();

        //[4] Send write request to node(adress:)
        if (auto *reply = modbusDevice->sendWriteRequest(writeUnit, _address)) {
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
            } else {// Broadcast replies return immediately
                reply->deleteLater();
            }
        } else {
            ui->textBrowser->append(tr("Write error: ") + modbusDevice->errorString());
        }
    }

}

void FanMotorUi::on_pushButton_InitSetG_clicked()//Set group fan Init parameter
{
    if(m_communication == CommunicationMode::Modbus){
        if (!modbusDevice)
            return;

        //[1] Init data
        QAbstractItemModel *__model = ui->table_settings->model();
        int _startAddress = ui->spinBox_InitAdress->minimum();//Server address
        int _lastAddress = ui->spinBox_InitAdress->maximum();
        int _pos = ui->spinBox_InitAdress->value()-ui->spinBox_InitAdress->minimum();//Position at container
        quint16 *buff = (quint16 *)&m_motors->at(_pos)->m_initSetttings;

        //[2] Get data from tableview
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


        //[3] Pack data to a QModbusDataUnit
        buff = (quint16 *)&m_motors->at(_pos)->m_initSetttings;
        const QModbusDataUnit::RegisterType table = QModbusDataUnit::HoldingRegisters;
        int startAddress = g_mSettingsRegisterAddress;
        int numberOfEntries = g_mSettingsRegisterCount;
        QModbusDataUnit writeUnit = QModbusDataUnit(table, startAddress, numberOfEntries);
        for (uint i = 0; i < writeUnit.valueCount(); i++) {

            writeUnit.setValue(i, *buff++);
        }
        qDebug() <<writeUnit.valueCount();

        //[4] Send write request to node(adress:i) at current group
        for(int i = _startAddress; i <= _lastAddress; i++){

            if (auto *reply = modbusDevice->sendWriteRequest(writeUnit, i)) {
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
                } else {// Broadcast replies return immediately
                    reply->deleteLater();
                }
            } else {
                ui->textBrowser->append(tr("Write error: ") + modbusDevice->errorString());
            }
        }
    }
}

void FanMotorUi::on_pushButton_InitSetA_clicked()//Set all fan Init parameter
{
    if(m_communication == CommunicationMode::Modbus){
        if (!modbusDevice)
            return;

        //[1] Init data
        QAbstractItemModel *__model = ui->table_settings->model();
        int _address = 0;//Broadcast address
        int _pos = ui->spinBox_InitAdress->value()-ui->spinBox_InitAdress->minimum();//Position at container
        quint16 *buff = (quint16 *)&m_motors->at(_pos)->m_initSetttings;

        //[2] Get data from tableview
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

        //[3] Pack data to a QModbusDataUnit
        buff = (quint16 *)&m_motors->at(_pos)->m_initSetttings;
        const QModbusDataUnit::RegisterType table = QModbusDataUnit::HoldingRegisters;
        int startAddress = g_mSettingsRegisterAddress;
        int numberOfEntries = g_mSettingsRegisterCount;
        QModbusDataUnit writeUnit = QModbusDataUnit(table, startAddress, numberOfEntries);
        for (uint i = 0; i < writeUnit.valueCount(); i++) {

            writeUnit.setValue(i, *buff++);
        }
        qDebug() <<writeUnit.valueCount();

        //[4] Send write request to node(adress:0) : Broadcast
        if (auto *reply = modbusDevice->sendWriteRequest(writeUnit, _address)) {
            if (!reply->isFinished()) {
            } else {// Broadcast replies return immediately
                reply->deleteLater();
            }
        } else {
            ui->textBrowser->append(tr("Write error: ") + modbusDevice->errorString());
        }
    }
}

void FanMotorUi::on_pushButton_InitRead_clicked()//Read fan Init parameter
{
    if(m_communication == CommunicationMode::Modbus){
        if (!modbusDevice)
            return;

        int startAddress = g_mSettingsRegisterAddress; //register address
        int numberOfEntries = g_mSettingsRegisterCount; //16bit register number
        qDebug() <<"Read number Of entries:"<<numberOfEntries;
        QModbusDataUnit _modbusDataUnit(QModbusDataUnit::HoldingRegisters, startAddress, numberOfEntries);

        if (auto *reply = modbusDevice->sendReadRequest(_modbusDataUnit, ui->spinBox_InitAdress->value())) {
            if (!reply->isFinished()){
                connect(reply, &QModbusReply::finished, this, &FanMotorUi::readInitReady);
            }
            else
                delete reply; // broadcast replies return immediately
        } else {
            ui->textBrowser->append(tr("Read error: ") + modbusDevice->errorString());
        }
    }
}

void FanMotorUi::readInitReady()//Read fan Init parameter Ready
{
    QModbusReply *reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;

    int __pos = ui->spinBox_InitAdress->value() - ui->spinBox_InitAdress->minimum();//Current motor pos of container

    //Reply no error
    if (reply->error() == QModbusDevice::NoError) {

        const QModbusDataUnit unit = reply->result();//Get reply data unit

        quint16 *buff;
        if(unit.startAddress() == g_mSettingsRegisterAddress){//Make sure is Init data
            buff = (quint16 *)&m_motors->at(__pos)->m_initSetttings;//Get current motor's initSetttings pointer
        }

        //Put data to current motor's initSetttings
        for (uint i = 0; i < unit.valueCount(); i++) {

            *buff++ = unit.value(i);

            const QString entry = tr("Address: %1, Value: %2").arg(QString::number(unit.startAddress()+i, 16))
                    .arg(QString::number(unit.value(i), 16));
            ui->textBrowser->append(entry);//Show out the data
        }

        m_motors->at(__pos)->m_communicationState =FanCommunicationState::m_connect;//Update communication state

    }
    //Reply protocol error
    else if (reply->error() == QModbusDevice::ProtocolError) {
        ui->textBrowser->append(tr("Read response error: %1 (Mobus exception: 0x%2)").
                                arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 16));
        m_motors->at(__pos)->m_communicationState =FanCommunicationState::m_comError;//Update communication state
    }
    //Reply timeout
    else {
        ui->textBrowser->append(tr("Read response error: %1 (code: 0x%2)").
                                arg(reply->errorString()).arg(reply->error(), -1, 16));
        m_motors->at(__pos)->m_communicationState =FanCommunicationState::m_disconnect;//Update communication state
    }

    m_motors->at(__pos)->update();//Update motor data to ui

    //Update data to settings table
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

    //For update sigle motor ui
    if(m_motors->at(__pos)->m_address == ui->spinBox_InitAdress->value()){
        if(!SigleMotorDialog::getS_Instance()->isHidden())
            emit updateSigleMotor(0);
    }

    reply->deleteLater();
}
/*Init parameter set and read fuction*/

/*Group and all motor monitor fuction*/
void FanMotorUi::on_groupmonitorButton_clicked()//Show monitor current group ui
{
    if(!m_currentGroup->m_monitorDialog)
        m_currentGroup->m_monitorDialog = new GroupMonitorDialog(&m_currentGroup->m_motors);

    m_currentGroup->m_monitorDialog->show(m_currentGroup->m_groupID, &m_currentGroup->m_motors);
}

void FanMotorUi::on_checkBox_monitorGSW_stateChanged(int arg1)//Open or close monitor group fan
{
    m_currentGroup->isMonitor = arg1;
    int _pos = ui->spinBox_InitAdress->value()-ui->spinBox_InitAdress->minimum();
    QMotor *_currentMotor = m_motors->at(_pos);

    foreach(QMotor *_motor, m_currentGroup->m_motors){
        if(_motor != _currentMotor)
            _motor->isMonitor = arg1;

    }
    if(isSigleMotorMonitor)
        _currentMotor->isMonitor = true;
    else
        _currentMotor->isMonitor = arg1;

    if(arg1){//Open

        if(!m_monitorState)//If not start monitor and arg1 > 0 , then start
            monitor_stateChanged(1);

    }
    else{//Close
        if(!ui->checkBox_monitorGSW->isChecked())//ALL close
            monitor_stateChanged(0);//close
    }
}

void FanMotorUi::on_checkBox_monitorASW_stateChanged(int arg1)//Open or close monitor all fan
{
    if(!m_groups)
        m_groups = homewindow::getInstance()->groups();

    int _pos = ui->spinBox_InitAdress->value()-ui->spinBox_InitAdress->minimum();
    QMotor *_currentMotor = m_motors->at(_pos);

    foreach(FanGroupInfo *group, *m_groups){
        group->isMonitor = arg1;
        foreach(QMotor *_motor, group->m_motors){
            if(_motor != _currentMotor)
                _motor->isMonitor = arg1;
        }
    }
    if(isSigleMotorMonitor)
        _currentMotor->isMonitor = true;
    else
        _currentMotor->isMonitor = arg1;

    if(m_communication == CommunicationMode::Modbus){
        monitor_stateChanged(arg1);
        ui->checkBox_monitorGSW->setChecked(arg1);
    }
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
    else{//tcp

    }
}

void FanMotorUi::monitor_stateChanged(int state)//Monitor state change (open or close)
{
    if(m_monitorState == (bool)state)
        return;

    m_monitorState = state;//Update monitor state

    //Start  monitor
    if(state){
        modbusDevice->setTimeout(g_responseTimeout);//Set response timeout 100ms
        modbusDevice->setNumberOfRetries(g_numberOfTry);//Set retry number 1
        QTimer::singleShot(0, [this]() { monitorTimer_update(); });
    }
    //Stop monitor
    else{
        modbusDevice->setTimeout(100);//Set response timeout 300ms
        modbusDevice->setNumberOfRetries(2);//Set retry number 2
    }
}

void FanMotorUi::monitorSigleStateChange(int state)
{
    int _pos = ui->spinBox_InitAdress->value()-ui->spinBox_InitAdress->minimum();
    QMotor *_currentMotor = m_motors->at(_pos);
    _currentMotor->isMonitor = state;
    isSigleMotorMonitor = state;
    qDebug()<<"isSigleMotorMonitor"<<isSigleMotorMonitor;

    if(state){
        if(m_monitorState == 1){//Already start

        }
        else{//Now Start
            modbusDevice->setTimeout(100);//Set response timeout 100ms
            modbusDevice->setNumberOfRetries(2);//Set retry number 1
            QTimer::singleShot(0, [this]() { monitorTimer_update(); });
        }
    }

}

void FanMotorUi::monitorTimer_update()//Monitor group and all timer update
{
    //[1] Wait until no timerout response
    if(m_monitorTimeroutCount > 0){
        //Delay some time , in case one cycle not complete yet
        QTimer::singleShot(m_monitorTimeroutCount*g_responseTimeout*g_numberOfTry + 50, [this]() { monitorTimer_update(); });
        m_monitorTimeroutCount = 0;
        return;
    }

    //[2] Get Groups pointer
    if(!m_groups)
        m_groups = homewindow::getInstance()->groups();

    int _pos = ui->spinBox_InitAdress->value()-ui->spinBox_InitAdress->minimum();
    QMotor *_currentMotor = m_motors->at(_pos);

    //[3] For ench motor , check state and send request
    m_monitorTimerPeriod = 0;
    m_monitorTimeroutCount = 0;
    int startAddress = g_mRealTimeRegisterAddress; //register address
    int numberOfEntries = g_mRealTimeRegisterCount; //16bit register number
    foreach(FanGroupInfo *_group, *m_groups){
        if(_group->isMonitor || (m_currentGroup == _group && isSigleMotorMonitor)){
            foreach(QMotor *_motor, _group->m_motors){

                if(_motor->isMonitor){
                    if (!modbusDevice)
                        return;

                    //[3.1] Pack data unit
                    QModbusDataUnit _modbusDataUnit(QModbusDataUnit::HoldingRegisters, startAddress, numberOfEntries);

                    if(!isSigleMotorMonitor && m_monitorState){
                        if (auto *reply = modbusDevice->sendReadRequest(_modbusDataUnit, _motor->m_address)) {
                            if (!reply->isFinished()){
                                connect(reply, &QModbusReply::finished, this, &FanMotorUi::monitorReadReady);
                            }
                            else
                                delete reply; // Broadcast replies return immediately
                        } else {
                            ui->textBrowser->append(tr("motor%1: Read error: ").arg(_motor->m_address) + modbusDevice->errorString());
                        }

                        m_monitorTimerPeriod += g_responseTimeout;//One motor request ,then period value add
                    }
                    else{
                        //[3.2] Current watch motor we need more information, change the number of entries
                        if(_motor->m_address == _currentMotor->m_address){
                            _modbusDataUnit.setValueCount(g_mRealTimeRegisterMoreCount);
                        }
                        else
                            _modbusDataUnit.setValueCount(g_mRealTimeRegisterStateCount);

                        //[3.3] Send read request to one motor
                        if (auto *reply = modbusDevice->sendReadRequest(_modbusDataUnit, _motor->m_address)) {
                            if (!reply->isFinished()){
                                connect(reply, &QModbusReply::finished, this, &FanMotorUi::monitorReadReady);
                            }
                            else
                                delete reply; // Broadcast replies return immediately
                        } else {
                            ui->textBrowser->append(tr("motor%1: Read error: ").arg(_motor->m_address) + modbusDevice->errorString());
                        }

                        m_monitorTimerPeriod += g_responseTimeout;//One motor request ,then period value add

                        //[3.4] If not current watch motor, we send a request to current watch motor
                        if(_motor->m_address != _currentMotor->m_address){

                            //Change the number of entries
                            _modbusDataUnit.setValueCount(g_mRealTimeRegisterMoreCount);

                            //Send read request to one motor
                            if (auto *reply = modbusDevice->sendReadRequest(_modbusDataUnit, _currentMotor->m_address)) {
                                if (!reply->isFinished()){
                                    connect(reply, &QModbusReply::finished, this, &FanMotorUi::monitorReadReady);
                                }
                                else
                                    delete reply; // Broadcast replies return immediately
                            } else {
                                ui->textBrowser->append(tr("motor%1: Read error: ").arg(_motor->m_address) + modbusDevice->errorString());
                            }

                            m_monitorTimerPeriod += g_responseTimeout;//One motor request ,then period value add
                        }

                    }

                }

            }
        }
    }

    //[4] IF not interrupted ,go on next period
    if(m_monitorState || isSigleMotorMonitor){
        QTimer::singleShot(m_monitorTimerPeriod*g_numberOfTry, [this]() { monitorTimer_update(); });
    }

}

void FanMotorUi::monitorReadReady()//Monitor group and all read ready
{
    QModbusReply *reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;
    int _motorAddress = reply->serverAddress() ;

    if(!m_groups)
        m_groups = homewindow::getInstance()->groups();


    QMotor *motor = Q_NULLPTR;
    foreach(FanGroupInfo *_group, *m_groups){//Get corresponding motor
        //Judge belong to which group
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


    if (reply->error() == QModbusDevice::NoError) {

        const QModbusDataUnit unit = reply->result();

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

    motor->update();//Update motor ui

    //For update sigle motor ui
    if(motor->m_address == ui->spinBox_InitAdress->value()){
        if(isSigleMotorMonitor)
            emit updateSigleMotor(1);
    }

    reply->deleteLater();

}
/*Group and all motor monitor fuction*/


/*Initialize fan / group / all motor fuction*/
void FanMotorUi::on_clearButton_clicked()
{
    ui->textBrowser->clear();
}

void FanMotorUi::on_initializeButton_clicked()
{
    if(m_communication == CommunicationMode::Modbus){
        if (!modbusDevice)
            return;

        int startAddress = g_mControllerRegisterAddress; //register address
        int numberOfEntries = g_mRatedRegisterCount; //16bit register number
        qDebug() <<"Read number Of entries:"<<numberOfEntries;
        QModbusDataUnit _modbusDataUnit(QModbusDataUnit::HoldingRegisters, startAddress, numberOfEntries);

        if (auto *reply = modbusDevice->sendReadRequest(_modbusDataUnit, ui->spinBox_InitAdress->value())) {
            if (!reply->isFinished()){
                connect(reply, &QModbusReply::finished, this, &FanMotorUi::readInitFGAReady);
            }
            else
                delete reply; // broadcast replies return immediately
        } else {
            ui->textBrowser->append(tr("Read error: ") + modbusDevice->errorString());
        }
    }

    if(!m_currentGroup->m_monitorDialog){
            m_currentGroup->m_monitorDialog = new GroupMonitorDialog(&m_currentGroup->m_motors);
    }
    if(m_currentGroup->m_monitorDialog->isHidden() && SigleMotorDialog::getS_Instance()->isHidden())
        m_currentGroup->m_monitorDialog->show(m_currentGroup->m_groupID, m_motors);
}

void FanMotorUi::on_initializeGButton_clicked()
{
    foreach(QMotor *_motor, m_currentGroup->m_motors){
        if (!modbusDevice)
            return;

        //Pack data unit
        int startAddress = g_mControllerRegisterAddress; //register address
        int numberOfEntries = g_mRatedRegisterCount; //16bit register number
        QModbusDataUnit _modbusDataUnit(QModbusDataUnit::HoldingRegisters, startAddress, numberOfEntries);

        //Send read request to one motor
        if (auto *reply = modbusDevice->sendReadRequest(_modbusDataUnit, _motor->m_address)) {
            if (!reply->isFinished()){
                connect(reply, &QModbusReply::finished, this, &FanMotorUi::readInitFGAReady);
            }
            else
                delete reply; // Broadcast replies return immediately
        } else {
            ui->textBrowser->append(tr("motor%1: Read error: ").arg(_motor->m_address) + modbusDevice->errorString());
        }
    }

    if(!m_currentGroup->m_monitorDialog){
            m_currentGroup->m_monitorDialog = new GroupMonitorDialog(&m_currentGroup->m_motors);
    }
    if(m_currentGroup->m_monitorDialog->isHidden())
        m_currentGroup->m_monitorDialog->show(m_currentGroup->m_groupID, m_motors);
}

void FanMotorUi::on_initializeAButton_clicked()
{
    //[1] Get Groups pointer
    if(!m_groups)
        m_groups = homewindow::getInstance()->groups();

    foreach(FanGroupInfo *_group, *m_groups){
        if(_group->isMonitor){
            foreach(QMotor *_motor, _group->m_motors){
                if (!modbusDevice)
                    return;

                //Pack data unit
                int startAddress = g_mControllerRegisterAddress; //register address
                int numberOfEntries = g_mRatedRegisterCount; //16bit register number
                QModbusDataUnit _modbusDataUnit(QModbusDataUnit::HoldingRegisters, startAddress, numberOfEntries);

                //Send read request to one motor
                if (auto *reply = modbusDevice->sendReadRequest(_modbusDataUnit, _motor->m_address)) {
                    if (!reply->isFinished()){
                        connect(reply, &QModbusReply::finished, this, &FanMotorUi::readInitFGAReady);
                    }
                    else
                        delete reply; // Broadcast replies return immediately
                } else {
                    ui->textBrowser->append(tr("motor%1: Read error: ").arg(_motor->m_address) + modbusDevice->errorString());
                }
            }

        }
    }

    if(!m_currentGroup->m_monitorDialog){
            m_currentGroup->m_monitorDialog = new GroupMonitorDialog(&m_currentGroup->m_motors);
    }
    if(m_currentGroup->m_monitorDialog->isHidden())
        m_currentGroup->m_monitorDialog->show(m_currentGroup->m_groupID, m_motors);
}

void FanMotorUi::readInitFGAReady()//Read fan Init parameter read ready
{
    QModbusReply *reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;

    int _motorAddress = reply->serverAddress() ;

    if(!m_groups)
        m_groups = homewindow::getInstance()->groups();

    QMotor *motor = Q_NULLPTR;
    foreach(FanGroupInfo *_group, *m_groups){//Get corresponding motor
        //Judge belong to which group
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

    if (reply->error() == QModbusDevice::NoError) {

        const QModbusDataUnit unit = reply->result();

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

    motor->update();//Update motor ui

    //For update sigle motor ui
    if(motor->m_address == ui->spinBox_InitAdress->value()){
        if(!SigleMotorDialog::getS_Instance()->isHidden())
            emit updateSigleMotor(2);
    }

    reply->deleteLater();
}
/*Initialize fan / group / all motor fuction*/


void FanMotorUi::onRunStateButtonClicked()//Run or stop fan / group / all
{
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    if(btn == ui->pushButton_startMotor || btn == ui->pushButton_stopMotor){

        if(m_communication == CommunicationMode::Modbus){
            if (!modbusDevice)
                return;

            //[1] Init data
            int _address = ui->spinBox_InitAdress->value();//Server address
            int _pos = ui->spinBox_InitAdress->value()-ui->spinBox_InitAdress->minimum();//Position at container
            if(btn == ui->pushButton_startMotor)
                m_motors->at(_pos)->m_motorController.m_runState = FanMotorState::m_run;
            else
                m_motors->at(_pos)->m_motorController.m_runState = FanMotorState::m_stop;
            quint16 *buff = (quint16 *)&m_motors->at(_pos)->m_motorController;
            buff += 2;

            //[2] Pack data to a QModbusDataUnit
            const QModbusDataUnit::RegisterType table = QModbusDataUnit::HoldingRegisters;
            int startAddress = g_mRealTimeRegisterAddress;
            int numberOfEntries = g_mRealTimeRegisterStateCount;
            QModbusDataUnit writeUnit = QModbusDataUnit(table, startAddress, numberOfEntries);
            for (uint i = 0; i < writeUnit.valueCount(); i++) {

                writeUnit.setValue(i, *buff++);
            }
            qDebug() <<writeUnit.valueCount();

            //[3] Send write request to node(adress:)
            if (auto *reply = modbusDevice->sendWriteRequest(writeUnit, _address)) {
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
                } else {// Broadcast replies return immediately
                    reply->deleteLater();
                }
            } else {
                ui->textBrowser->append(tr("Write error: ") + modbusDevice->errorString());
            }
        }

    }
    else if(btn == ui->pushButton_startMotor_G || btn == ui->pushButton_stopMotor_G){

        if(m_communication == CommunicationMode::Modbus){
            if (!modbusDevice)
                return;

            //[1] Init data
            int _startAddress = ui->spinBox_InitAdress->minimum();//Server address
            int _lastAddress = ui->spinBox_InitAdress->maximum();
            int _pos = ui->spinBox_InitAdress->value()-ui->spinBox_InitAdress->minimum();//Position at container
            if(btn == ui->pushButton_startMotor_G)
                m_motors->at(_pos)->m_motorController.m_runState = FanMotorState::m_run;
            else
                m_motors->at(_pos)->m_motorController.m_runState = FanMotorState::m_stop;
            quint16 *buff = (quint16 *)&m_motors->at(_pos)->m_motorController;
            buff += 2;

            //[2] Pack data to a QModbusDataUnit
            const QModbusDataUnit::RegisterType table = QModbusDataUnit::HoldingRegisters;
            int startAddress = g_mRealTimeRegisterAddress;
            int numberOfEntries = g_mRealTimeRegisterStateCount;
            QModbusDataUnit writeUnit = QModbusDataUnit(table, startAddress, numberOfEntries);
            for (uint i = 0; i < writeUnit.valueCount(); i++) {

                writeUnit.setValue(i, *buff++);
            }
            qDebug() <<writeUnit.valueCount();

            //[3] Send write request to node(adress:i) at current group
            for(int i = _startAddress; i <= _lastAddress; i++){

                if (auto *reply = modbusDevice->sendWriteRequest(writeUnit, i)) {
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
                    } else {// Broadcast replies return immediately
                        reply->deleteLater();
                    }
                } else {
                    ui->textBrowser->append(tr("Write error: ") + modbusDevice->errorString());
                }
            }
        }

    }
    else if(btn == ui->pushButton_startMotor_A || btn == ui->pushButton_stopMotor_A){
        if(m_communication == CommunicationMode::Modbus){
            if (!modbusDevice)
                return;

            //[1] Init data
            int _address = 0;//Broadcast address
            int _pos = ui->spinBox_InitAdress->value()-ui->spinBox_InitAdress->minimum();//Position at container
            if(btn == ui->pushButton_startMotor_A)
                m_motors->at(_pos)->m_motorController.m_runState = FanMotorState::m_run;
            else
                m_motors->at(_pos)->m_motorController.m_runState = FanMotorState::m_stop;
            quint16 *buff = (quint16 *)&m_motors->at(_pos)->m_motorController;
            buff += 2;

            //[2] Pack data to a QModbusDataUnit
            const QModbusDataUnit::RegisterType table = QModbusDataUnit::HoldingRegisters;
            int startAddress = g_mRealTimeRegisterAddress;
            int numberOfEntries = g_mRealTimeRegisterStateCount;
            QModbusDataUnit writeUnit = QModbusDataUnit(table, startAddress, numberOfEntries);
            for (uint i = 0; i < writeUnit.valueCount(); i++) {

                writeUnit.setValue(i, *buff++);
            }
            qDebug() <<writeUnit.valueCount();

            //[3] Send write request to node(adress:0) : Broadcast
            if (auto *reply = modbusDevice->sendWriteRequest(writeUnit, _address)) {
                if (!reply->isFinished()) {
                } else {// Broadcast replies return immediately
                    reply->deleteLater();
                }
            } else {
                ui->textBrowser->append(tr("Write error: ") + modbusDevice->errorString());
            }
        }

    }
}

void FanMotorUi::onMotorStartButtonClicked()//For SigleMotorDialog ui
{
    ui->pushButton_startMotor->clicked();
}

void FanMotorUi::onMotorStopButtonClicked()//For SigleMotorDialog ui
{
    ui->pushButton_stopMotor->clicked();
}

void FanMotorUi::onFanButtonclicked()//Show SigleMotorDialog ui
{
    QPushButton* btn = qobject_cast<QPushButton*>(sender());

    if(!m_motors)
        return;

    for(int i = 0; i < m_motorButtons.count(); i++){
        if(btn == m_motorButtons.at(i)){

            ui->spinBox_InitAdress->setValue(m_motors->at(i)->m_address);
            SigleMotorDialog::getS_Instance()->show(m_motors->at(i));

            if(m_communication == CommunicationMode::Modbus ){
                if(!modbusDevice)
                    emit updateSigleMotorState(0);
                else{
                    bool connected = (modbusDevice->state() != QModbusDevice::UnconnectedState);
                    emit updateSigleMotorState(connected);
                }
            }
        }

    }
}

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

void FanMotorUi::onSetPI()
{
    if(m_communication == CommunicationMode::Modbus){
        if (!modbusDevice)
            return;

        //[1] Init data

        int _address = ui->spinBox_InitAdress->value();//Server address
        int _pos = ui->spinBox_InitAdress->value()-ui->spinBox_InitAdress->minimum();//Position at container
        quint16 *buff = (quint16 *)&m_motors->at(_pos)->m_PIPara;

        //[3] Pack data to a QModbusDataUnit
        const QModbusDataUnit::RegisterType table = QModbusDataUnit::HoldingRegisters;
        int startAddress = g_mPIParaRegisterAddress;
        int numberOfEntries = g_mPIParaRegisterMoreCount;
        QModbusDataUnit writeUnit = QModbusDataUnit(table, startAddress, numberOfEntries);
        for (uint i = 0; i < writeUnit.valueCount(); i++) {

            writeUnit.setValue(i, *buff++);
        }
        qDebug() <<writeUnit.valueCount();

        //[4] Send write request to node(adress:)
        if (auto *reply = modbusDevice->sendWriteRequest(writeUnit, _address)) {
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
            } else {// Broadcast replies return immediately
                reply->deleteLater();
            }
        } else {
            ui->textBrowser->append(tr("Write error: ") + modbusDevice->errorString());
        }
    }
}

void FanMotorUi::onReadPI()
{
    if(m_communication == CommunicationMode::Modbus){
        if (!modbusDevice)
            return;

        int startAddress = g_mPIParaRegisterAddress; //register address
        int numberOfEntries = g_mPIParaRegisterMoreCount; //16bit register number
        qDebug() <<"Read number Of entries:"<<numberOfEntries;
        QModbusDataUnit _modbusDataUnit(QModbusDataUnit::HoldingRegisters, startAddress, numberOfEntries);

        if (auto *reply = modbusDevice->sendReadRequest(_modbusDataUnit, ui->spinBox_InitAdress->value())) {
            if (!reply->isFinished()){
                connect(reply, &QModbusReply::finished, this, &FanMotorUi::readPIReady);
            }
            else
                delete reply; // broadcast replies return immediately
        } else {
            ui->textBrowser->append(tr("Read error: ") + modbusDevice->errorString());
        }
    }
}

void FanMotorUi::readPIReady()//Read fan Init parameter Ready
{
    QModbusReply *reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;

    int __pos = ui->spinBox_InitAdress->value() - ui->spinBox_InitAdress->minimum();//Current motor pos of container

    //Reply no error
    if (reply->error() == QModbusDevice::NoError) {

        const QModbusDataUnit unit = reply->result();//Get reply data unit

        quint16 *buff;
        if(unit.startAddress() == g_mPIParaRegisterAddress){//Make sure is Init data
            buff = (quint16 *)&m_motors->at(__pos)->m_PIPara;//Get current motor's initSetttings pointer
        }

        //Put data to current motor's initSetttings
        for (uint i = 0; i < unit.valueCount(); i++) {

            *buff++ = unit.value(i);

            const QString entry = tr("Address: %1, Value: %2").arg(QString::number(unit.startAddress()+i, 16))
                    .arg(QString::number(unit.value(i), 16));
            ui->textBrowser->append(entry);//Show out the data
        }

        m_motors->at(__pos)->m_communicationState =FanCommunicationState::m_connect;//Update communication state

    }
    //Reply protocol error
    else if (reply->error() == QModbusDevice::ProtocolError) {
        ui->textBrowser->append(tr("Read response error: %1 (Mobus exception: 0x%2)").
                                arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 16));
        m_motors->at(__pos)->m_communicationState =FanCommunicationState::m_comError;//Update communication state
    }
    //Reply timeout
    else {
        ui->textBrowser->append(tr("Read response error: %1 (code: 0x%2)").
                                arg(reply->errorString()).arg(reply->error(), -1, 16));
        m_motors->at(__pos)->m_communicationState =FanCommunicationState::m_disconnect;//Update communication state
    }

    m_motors->at(__pos)->update();//Update motor data to ui

    //For update sigle motor ui
    if(m_motors->at(__pos)->m_address == ui->spinBox_InitAdress->value()){
        if(!SigleMotorDialog::getS_Instance()->isHidden())
            emit updateSigleMotor(3);
    }

    reply->deleteLater();
}

