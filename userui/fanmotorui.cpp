#include "fanmotorui.h"
#include "ui_fanmotorui.h"

#include <QModbusTcpClient>
#include <QModbusRtuSerialMaster>
#include <QStandardItemModel>
#include <QTimer>

#include<qdebug.h>
#include "lamp/qcw_indicatorlamp.h"

#include"fanmotor/qmotor.h"
#include "userui/mplotui.h"

FanMotorUi *FanMotorUi::s_Instance = nullptr;


void FanMotorUi::heartbeatError(CO_Data *d, unsigned char heartbeatID)
{
    UNS8 __address = heartbeatID - 1;

    m_motors.at(__address)->m_communicationState =FanCommunicationState::m_disconnect;

    m_motors.at(__address)->update();

    if(heartbeatID == m_realTimeServerAddress){
        ui->lamp_comState->setLampState(m_motors.at(__address)->m_commLamp->getLampState());
    }

    qDebug()<<d->nodeState<<"heartbeatError"<<heartbeatID;
}
void master_heartbeatError(CO_Data* d, UNS8 heartbeatID)
{
    FanMotorUi::getS_Instance()->heartbeatError(d, heartbeatID);

}

void master_post_sync(CO_Data* d)
{
//    qDebug()<<d->nodeState;
}

void FanMotorUi::post_SlaveStateChange(CO_Data* d, UNS8 nodeId, e_nodeState newNodeState)
{
    UNS8 __address = nodeId - 1;
    if(newNodeState == Disconnected){
        m_motors.at(__address)->m_communicationState =FanCommunicationState::m_disconnect;
    }
    else{
        m_motors.at(__address)->m_communicationState =FanCommunicationState::m_connect;
    }

    m_motors.at(__address)->update();

    if(nodeId == m_realTimeServerAddress){
        ui->lamp_comState->setLampState(m_motors.at(__address)->m_commLamp->getLampState());
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


FanMotorUi::FanMotorUi(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FanMotorUi)
    ,m_masterBoard{new s_BOARD{}}
    ,m_canThread{CanThread::getInstance()}
{
    ui->setupUi(this);

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


    m_masterBoard->busname = const_cast<char *>("master");
    m_masterBoard->baudrate = const_cast<char *>("125000");

    m_pollingState = PollingState::Stop;

    m_CANopenStart = false;

    ui->startButton->setEnabled(false);
    ui->searchButton->setEnabled(false);
    ui->pushButton_read->setEnabled(false);
    ui->pushButton_write->setEnabled(false);
    ui->pushButton_startMotor->setEnabled(false);
    ui->pushButton_stopMotor->setEnabled(false);
    ui->disconnectButton->setEnabled(false);

    m_canUi = CANUi::getS_Instance();

    m_communication = Communication::Init;
    m_modbusUi = ModbusUi::getInstance();
    modbusDevice = m_modbusUi->getModbusDevice();
    on_radioButton_modbus_clicked();


    m_realTimeServerAddress = ui->spinBox_currentaddress->value();
    m_motorNum = ui->spinBox_motorNum->value();
    m_startServerAddress = 1;

    for(int i = 0; i< m_motorNum; i++){
        m_motors.push_back(new QMotor(i+m_startServerAddress));
        ui->table_realtime->setCellWidget(i, 0, m_motors.at(i)->m_motorAddressLabel);
        ui->table_realtime->setCellWidget(i, 1, m_motors.at(i)->m_ratedPowerLCD);
        ui->table_realtime->setCellWidget(i, 2, m_motors.at(i)->m_targetPowerLCD);
        ui->table_realtime->setCellWidget(i, 3, m_motors.at(i)->m_nowPowerLCD);
        ui->table_realtime->setCellWidget(i, 4, m_motors.at(i)->m_ratedSpeedLCD);
        ui->table_realtime->setCellWidget(i, 5, m_motors.at(i)->m_speedRefLCD);
        ui->table_realtime->setCellWidget(i, 6, m_motors.at(i)->m_speedFbkLCD);
        ui->table_realtime->setCellWidget(i, 7, m_motors.at(i)->m_runLamp);
        ui->table_realtime->setCellWidget(i, 8, m_motors.at(i)->m_commLamp);

    }

    m_pollingTimer = new QTimer(this);
    connect(m_pollingTimer, SIGNAL(timeout()), this, SLOT(pollingTimerUpdate()));

    if(m_communication == Communication::Modbus){

    }

    m_motors.at(0)->m_communicationState =FanCommunicationState::m_connect;
    m_motors.at(1)->m_communicationState =FanCommunicationState::m_connect;
    m_motors.at(2)->m_communicationState =FanCommunicationState::m_connect;

    MPlotUi*__mPlotUi = MPlotUi::getInstance();
    connect(this, &FanMotorUi::updatePlotUi, __mPlotUi, &MPlotUi::realtimeDataSlot);

}

void FanMotorUi::on_radioButton_modbus_clicked()
{
    if(m_communication != Communication::Modbus){

        connect(ui->connectButton, SIGNAL(clicked()),
                m_modbusUi, SLOT(on_connectButton_clicked()));
        connect(ui->disconnectButton, SIGNAL(clicked()),
                m_modbusUi, SLOT(on_connectButton_clicked()));
        connect(modbusDevice, &QModbusClient::stateChanged,
                this, &FanMotorUi::onModebusStateChanged);
    }

    m_communication = Communication::Modbus;

}

void FanMotorUi::on_radioButton_can_clicked()
{
    if(m_communication == Communication::Modbus){

        disconnect(ui->connectButton, SIGNAL(clicked()),
                   m_modbusUi, SLOT(on_connectButton_clicked()));
        disconnect(ui->disconnectButton, SIGNAL(clicked()),
                   m_modbusUi, SLOT(on_connectButton_clicked()));
        disconnect(modbusDevice, &QModbusClient::stateChanged,
                   this, &FanMotorUi::onModebusStateChanged);
    }

    m_communication = Communication::CANbus;
}

void FanMotorUi::on_radioButton_tcp_clicked()
{
    if(m_communication == Communication::Modbus){

        disconnect(ui->connectButton, SIGNAL(clicked()),
                   m_modbusUi, SLOT(on_connectButton_clicked()));
        disconnect(ui->disconnectButton, SIGNAL(clicked()),
                   m_modbusUi, SLOT(on_connectButton_clicked()));

        disconnect(modbusDevice, &QModbusClient::stateChanged,
                   this, &FanMotorUi::onModebusStateChanged);
    }

    m_communication = Communication::Tcp;
}

void FanMotorUi::on_connectButton_clicked()//open
{
    if(m_communication == Communication::CANbus){

        m_canUi->resetCAN();

        if(!canOpen(m_masterBoard, &master_Data)){
            ui->textBrowser->append("open failed !");
            return;
        }

        ui->textBrowser->append("open success !");

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

        m_canUi->setEnabled(false);//disable canui

        bool connected = true;
        ui->connectButton->setEnabled(!connected);
        ui->disconnectButton->setEnabled(connected);

        ui->startButton->setEnabled(connected);
        ui->searchButton->setEnabled(!connected);
        ui->pushButton_read->setEnabled(connected);
        ui->pushButton_write->setEnabled(connected);
        ui->pushButton_startMotor->setEnabled(connected);
        ui->pushButton_stopMotor->setEnabled(connected);

        ui->groupBox_comState->setEnabled(!connected);

    }
    else if(m_communication == Communication::Tcp){

    }
    else{

    }

}

void FanMotorUi::on_disconnectButton_clicked()//close
{
    if(m_communication == Communication::CANbus){

        if(m_CANopenStart){
            on_startButton_clicked();
        }

        m_canThread->mStop();               //then close thread
        disconnect(m_canThread, SIGNAL(message(QString)),
                this, SLOT(messageShow(QString)));

        disconnect(m_canThread, SIGNAL(message(QCanBusFrame)),
                this, SLOT(messageHandle(QCanBusFrame)));

        StopTimerLoop(&Exit);

        canClose(&master_Data);            //final close canport

        m_canUi->setEnabled(true);

        bool connected = false;
        ui->connectButton->setEnabled(!connected);
        ui->disconnectButton->setEnabled(connected);

        ui->startButton->setEnabled(connected);
        ui->searchButton->setEnabled(connected);
        ui->pushButton_read->setEnabled(connected);
        ui->pushButton_write->setEnabled(connected);
        ui->pushButton_startMotor->setEnabled(connected);
        ui->pushButton_stopMotor->setEnabled(connected);

        ui->groupBox_comState->setEnabled(!connected);

    }
    else if(m_communication == Communication::Tcp){

    }
    else{

    }
}

void FanMotorUi::onModebusStateChanged(int state)
{
    bool connected = (state != QModbusDevice::UnconnectedState);

    ui->connectButton->setEnabled(!connected);
    ui->disconnectButton->setEnabled(connected);

    ui->startButton->setEnabled(connected);
    ui->searchButton->setEnabled(connected);
    ui->pushButton_read->setEnabled(connected);
    ui->pushButton_write->setEnabled(connected);
    ui->pushButton_startMotor->setEnabled(connected);
    ui->pushButton_stopMotor->setEnabled(connected);

    ui->groupBox_comState->setEnabled(!connected);

}

void FanMotorUi::messageShow(const QString &s)
{

    ui->textBrowser->append(s);
}

void FanMotorUi::messageHandle(const QCanBusFrame &frame)//can frame handle
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

FanMotorUi::~FanMotorUi()
{
    delete ui;
    delete m_pollingTimer;
}

QModbusDataUnit FanMotorUi::readRequest() const
{
    int startAddress = g_mSettingsRegisterAddress; //register address

    int numberOfEntries = sizeof(FanMotorSettings) >> 1; //16bit register number
    qDebug() <<numberOfEntries;

    return QModbusDataUnit(QModbusDataUnit::HoldingRegisters, startAddress, numberOfEntries);
}
void FanMotorUi::on_pushButton_read_clicked()//read settings
{
    if (!modbusDevice)
        return;

    if (auto *reply = modbusDevice->sendReadRequest(readRequest(), ui->spinBox_currentaddress->value())) {
        if (!reply->isFinished()){
            connect(reply, &QModbusReply::finished, this, &FanMotorUi::readReady);
        }
        else
            delete reply; // broadcast replies return immediately
    } else {
        ui->textBrowser->append(tr("Read error: ") + modbusDevice->errorString());
    }
}
void FanMotorUi::readReady()
{
    QModbusReply *reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;

    int __currentAddress = reply->serverAddress() ;
    int __address = __currentAddress - m_startServerAddress;

    if (reply->error() == QModbusDevice::NoError) {

        const QModbusDataUnit unit = reply->result();

        quint16 *buff;
        if(unit.startAddress() == g_mSettingsRegisterAddress){//setting read
            buff = (quint16 *)&m_motors.at(__address)->m_initSetttings;
        }
        else if(unit.startAddress() == g_mControllerRegisterAddress){//single motor mode
            buff = (quint16 *)&m_motors.at(__address)->m_motorController;
        }
        else if(unit.startAddress() == g_mRealTimeRegisterAddress){//multi motor mode
            buff = (quint16 *)&m_motors.at(__address)->m_motorController;
            buff += g_mRatedRegisterCount;
        }
        else if(unit.startAddress() == g_mPIParaRegisterAddress){//pi read
            buff = (quint16 *)&m_motors.at(__address)->m_PIPara;
        }


        for (uint i = 0; i < unit.valueCount(); i++) {
            const QString entry = tr("Address: %1, Value: %2").arg(QString::number(unit.startAddress()+i, 16))
                    .arg(QString::number(unit.value(i), 16));
            ui->textBrowser->append(entry);

            *buff++ = unit.value(i);

        }

        m_motors.at(__address)->m_communicationState =FanCommunicationState::m_connect;

    } else if (reply->error() == QModbusDevice::ProtocolError) {

        ui->textBrowser->append(tr("Read response error: %1 (Mobus exception: 0x%2)").
                                arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 16));

        m_motors.at(__address)->m_communicationState =FanCommunicationState::m_comError;
    } else {

        ui->textBrowser->append(tr("Read response error: %1 (code: 0x%2)").
                                arg(reply->errorString()).arg(reply->error(), -1, 16));

        m_motors.at(__address)->m_communicationState =FanCommunicationState::m_disconnect;

    }

    if(m_currentServerAddress == __currentAddress){

        if(m_pollingState == PollingState::MultiMotor){//multi motor polling...

            qDebug()<<"multi motor polling...";
            for(m_currentServerAddress++;m_currentServerAddress < m_motorNum + m_startServerAddress; \
                m_currentServerAddress++){
                if(m_motors.at(m_currentServerAddress-m_startServerAddress)->m_communicationState == \
                        FanCommunicationState::m_connect){
                    sendOnePolling(m_currentServerAddress);
                    break;
                }
            }

        }
        else if(m_pollingState == PollingState::Searching){//searching...

            qDebug()<<"searching...";
            for(m_currentServerAddress++;m_currentServerAddress < m_motorNum + m_startServerAddress;\
                m_currentServerAddress++){
                sendOnePolling(m_currentServerAddress);
                break;
            }
            if(m_currentServerAddress >= m_motorNum + m_startServerAddress){
                m_pollingState = PollingState::Stop;
            }

        }
    }

    m_motors.at(__address)->update();

    if(m_realTimeServerAddress == __currentAddress){
        ui->lamp_comState->setLampState(m_motors.at(__address)->m_commLamp->getLampState());
        ui->lamp_runState->setLampState(m_motors.at(__address)->m_runLamp->getLampState());
        ui->lcdNumber_targetPower->display(m_motors.at(__address)->m_motorController.m_targetpower);
        ui->lcdNumber_nowPower->display(m_motors.at(__address)->m_motorController.m_nowpower);
        ui->lcdNumber_speedRef->display(m_motors.at(__address)->m_motorController.m_speedRef);
        ui->lcdNumber_speedFbk->display(m_motors.at(__address)->m_motorController.m_speedFbk);

        if(m_pollingState == PollingState::Stop){
            QAbstractItemModel *__model = ui->table_settings->model();
            quint16 *__buffPtr = (quint16 *)&m_motors.at(__address)->m_initSetttings;
            for(unsigned char i = 0; i<3; i++){
                for(unsigned char j = 1; j<5; j++){
                    __model->setData(__model->index(i,j), *__buffPtr);
                    __buffPtr++;
                }
            }
        }
        else if(m_pollingState == PollingState::SingleMotor){
            emit updatePlotUi(m_motors.at(__address)->m_motorController);
            qDebug()<<"emit.....";
        }
    }

    reply->deleteLater();
}


QModbusDataUnit FanMotorUi::writeRequest() const
{
    const QModbusDataUnit::RegisterType table = QModbusDataUnit::HoldingRegisters;

    int startAddress = g_mSettingsRegisterAddress;

    int numberOfEntries = sizeof(FanMotorSettings) >> 1;
    return QModbusDataUnit(table, startAddress, numberOfEntries);
}

void FanMotorUi::on_pushButton_write_clicked()//write settings
{
    if (!modbusDevice)
        return;

    QAbstractItemModel *__model = ui->table_settings->model();
    int __address = ui->spinBox_currentaddress->value()-m_startServerAddress;
    quint16 *buff = (quint16 *)&m_motors.at(__address)->m_initSetttings;

    //get data from tableview
    for(unsigned char i = 0; i<3; i++){
        for(unsigned char j = 1; j<5; j++){
            *buff = (quint16)__model->data(__model->index(i, j)).toInt();
            buff++;
        }
    }

    QModbusDataUnit writeUnit = writeRequest();
    buff = (quint16 *)&m_motors.at(__address)->m_initSetttings;

    for (uint i = 0; i < writeUnit.valueCount(); i++) {

        writeUnit.setValue(i, *buff++);
    }
    qDebug() <<writeUnit.valueCount();
    if (auto *reply = modbusDevice->sendWriteRequest(writeUnit, ui->spinBox_currentaddress->value())) {
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
        } else {
            // broadcast replies return immediately
            reply->deleteLater();
        }
    } else {
        ui->textBrowser->append(tr("Write error: ") + modbusDevice->errorString());
    }
}

void FanMotorUi::on_searchButton_clicked()
{
    //Searching mode start
    m_pollingState = PollingState::Searching;
    if(m_pollingTimer->isActive()){
        m_pollingTimer->stop();
        ui->startButton->setText(QStringLiteral("   start   "));
    }
    m_currentServerAddress = m_startServerAddress;
    sendOnePolling(m_currentServerAddress);
}

void FanMotorUi::on_startButton_clicked()
{
    if(m_communication == Communication::Modbus){
        //Multi motor mode start
        if(!m_pollingTimer->isActive()){

            m_pollingState = PollingState::MultiMotor;

            m_currentServerAddress = m_startServerAddress;
            for(m_currentServerAddress;m_currentServerAddress < m_motorNum + m_startServerAddress; \
                m_currentServerAddress++){
                if(m_motors.at(m_currentServerAddress-m_startServerAddress)->m_communicationState == \
                        FanCommunicationState::m_connect){
                    sendOnePolling(m_currentServerAddress);
                    break;
                }
            }
            m_pollingTimer->start(ui->spinBox_timePeriod->value());

            ui->startButton->setText(QStringLiteral("stop "));
            ui->pushButton_startMotor->setEnabled(false);
            ui->pushButton_stopMotor->setEnabled(false);

            ui->spinBox_motorNum->setEnabled(false);
        }
        else{
            m_pollingState = PollingState::Stop;
            m_pollingTimer->stop();
            ui->startButton->setText(QStringLiteral("start"));
            ui->pushButton_startMotor->setEnabled(true);
            ui->pushButton_stopMotor->setEnabled(true);

            ui->spinBox_motorNum->setEnabled(true);
        }
    }
    else if(m_communication == Communication::CANbus){
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
            ui->startButton->setText(QStringLiteral("stop "));
            ui->pushButton_startMotor->setEnabled(false);
            ui->pushButton_stopMotor->setEnabled(false);

            ui->spinBox_motorNum->setEnabled(false);
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
            ui->startButton->setText(QStringLiteral("start"));
            ui->pushButton_startMotor->setEnabled(true);
            ui->pushButton_stopMotor->setEnabled(true);

            ui->spinBox_motorNum->setEnabled(true);
        }
    }
    else{//tcp

    }

}

void FanMotorUi::on_pushButton_startMotor_clicked()
{
    m_pollingState = PollingState::SingleMotor;
    sendOnePolling(ui->spinBox_currentaddress->value());
    m_pollingTimer->start(ui->spinBox_pollingPeriod->value());

    ui->startButton->setEnabled(false);
}

void FanMotorUi::on_pushButton_stopMotor_clicked()
{
    m_pollingState = PollingState::Stop;
    m_pollingTimer->stop();

    ui->startButton->setEnabled(true);
}

void FanMotorUi::pollingTimerUpdate()
{
    qDebug()<<"timer update";

    if (!modbusDevice){
        return;
    }

    if(m_pollingState == PollingState::SingleMotor){
        sendOnePolling(ui->spinBox_currentaddress->value());
    }
    else{
        m_currentServerAddress = m_startServerAddress;
        for(m_currentServerAddress;m_currentServerAddress < m_motorNum + m_startServerAddress; \
            m_currentServerAddress++){
            if(m_motors.at(m_currentServerAddress-m_startServerAddress)->m_communicationState == \
                    FanCommunicationState::m_connect){
                sendOnePolling(m_currentServerAddress);
                break;
            }
        }
    }

}

void FanMotorUi::sendOnePolling(int address)
{
    qDebug()<<"send one polling";
    if (!modbusDevice){
        return;
    }

    int startAddress;

    int numberOfEntries;

    if(m_pollingState == PollingState::MultiMotor){
        startAddress = g_mRealTimeRegisterAddress;
        numberOfEntries  = g_mRealTimeRegisterCount;
    }
    else if(m_pollingState == PollingState::Searching){
        startAddress = g_mControllerRegisterAddress;
        numberOfEntries  = g_mRatedRegisterCount;
    }
    else{//single motor mode
        startAddress = g_mRealTimeRegisterAddress;
        numberOfEntries  = g_mRealTimeRegisterMoreCount;
    }

    QModbusDataUnit __dataUnit(QModbusDataUnit::HoldingRegisters, startAddress, numberOfEntries);

    if (auto *reply = modbusDevice->sendReadRequest(__dataUnit, address)) {
        if (!reply->isFinished()){
            connect(reply, &QModbusReply::finished, this, &FanMotorUi::readReady);
        }
        else
            delete reply; // broadcast replies return immediately
    } else {
        qDebug() << "error:" <<modbusDevice->errorString() ;
        ui->textBrowser->append(tr("error: ") + modbusDevice->errorString());
    }
}

void FanMotorUi::on_spinBox_motorNum_valueChanged(int arg1)
{
    m_motorNum = arg1;

    while(m_motors.count() < m_motorNum){
        int __motorCount = m_motors.count();
        m_motors.push_back(new QMotor(__motorCount+m_startServerAddress));
        ui->table_realtime->setCellWidget(__motorCount, 0, m_motors.back()->m_motorAddressLabel);
        ui->table_realtime->setCellWidget(__motorCount, 1, m_motors.back()->m_ratedPowerLCD);
        ui->table_realtime->setCellWidget(__motorCount, 2, m_motors.back()->m_targetPowerLCD);
        ui->table_realtime->setCellWidget(__motorCount, 3, m_motors.back()->m_nowPowerLCD);
        ui->table_realtime->setCellWidget(__motorCount, 4, m_motors.back()->m_ratedSpeedLCD);
        ui->table_realtime->setCellWidget(__motorCount, 5, m_motors.back()->m_speedRefLCD);
        ui->table_realtime->setCellWidget(__motorCount, 6, m_motors.back()->m_speedFbkLCD);
        ui->table_realtime->setCellWidget(__motorCount, 7, m_motors.back()->m_runLamp);
        ui->table_realtime->setCellWidget(__motorCount, 8, m_motors.back()->m_commLamp);
    }

    while(m_motors.count() > m_motorNum){
        int __motorCount = m_motors.count()-1;
        ui->table_realtime->removeCellWidget(__motorCount, 0);
        ui->table_realtime->removeCellWidget(__motorCount, 1);
        ui->table_realtime->removeCellWidget(__motorCount, 2);
        ui->table_realtime->removeCellWidget(__motorCount, 3);
        ui->table_realtime->removeCellWidget(__motorCount, 4);
        ui->table_realtime->removeCellWidget(__motorCount, 5);
        ui->table_realtime->removeCellWidget(__motorCount, 6);
        ui->table_realtime->removeCellWidget(__motorCount, 7);
        ui->table_realtime->removeCellWidget(__motorCount, 8);
        m_motors.pop_back();

    }
}

void FanMotorUi::on_spinBox_currentaddress_valueChanged(int arg1)
{
    m_realTimeServerAddress = arg1;
}


FanMotorUi *FanMotorUi::getS_Instance()
{
    if(!s_Instance)
    {
        s_Instance = new FanMotorUi;
    }
    return s_Instance;
}


