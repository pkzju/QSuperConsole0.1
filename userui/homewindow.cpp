#include "homewindow.h"
#include "ui_homewindow.h"

#include <QPushButton>

#include "mainwindow/mainwindow.h"
#include "userui/fanmotorui.h"
#include "dialogs/siglegroupdialog.h"

#include <QDebug>

extern CO_Data master_Data;

const int gGroupnum = 9; //Number of all groups
const int gMotorMaxnum = 15;


homewindow *homewindow::s_Instance = Q_NULLPTR;

homewindow::homewindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::homewindow)
  , mCurrentGroupInfo{Q_NULLPTR}
  , modbusDevice(new FModbusTcpServer(this))
  , modbusTcpClient(Q_NULLPTR)
{
    ui->setupUi(this);
    qDebug("homewindow init");

    //![1] Read settings from file: "/SuperConsole.ini"
    QSettings settings(QDir::currentPath() + "/SuperConsole.ini", QSettings::IniFormat);
    settings.beginGroup("homewindow");
    QString _hostAddressAndport = settings.value("HostAddressAndPort", "127.0.0.1:6474").toString();
    settings.endGroup();
    ui->portEdit->setText(_hostAddressAndport);

    {//Use a container to manage groups
        m_groupBtn.append(ui->pushButton_group1);
        m_groupBtn.append(ui->pushButton_group2);
        m_groupBtn.append(ui->pushButton_group3);
        m_groupBtn.append(ui->pushButton_group4);
        m_groupBtn.append(ui->pushButton_group5);
        m_groupBtn.append(ui->pushButton_group6);
        m_groupBtn.append(ui->pushButton_group7);
        m_groupBtn.append(ui->pushButton_group8);
        m_groupBtn.append(ui->pushButton_group9);

        on_spinBox_groupNum_valueChanged(ui->spinBox_groupNum->value());//Init group number
    }

    for(int i = 0; i < gGroupnum; i++){//All group buttons use one slot fuction
        connect(m_groupBtn.at(i), SIGNAL(clicked(bool)), this, SLOT(button_group_clicked()));
    }

    connect(modbusDevice, &QModbusServer::stateChanged,
            this, &homewindow::onStateChanged);
    connect(modbusDevice, &QModbusServer::errorOccurred,
            this, &homewindow::handleDeviceError);

}

homewindow::~homewindow()
{

    //! Save settings to file: "/SuperConsole.ini"
    QSettings settings(QDir::currentPath() + "/SuperConsole.ini", QSettings::IniFormat);
    qDebug(qPrintable(QDir::currentPath() + "/SuperConsole.ini"+ " group:homewindow") );
    settings.beginGroup("homewindow");
    settings.setValue("HostAddressAndPort", ui->portEdit->text());
    settings.endGroup();

    if(modbusDevice){
        disconnect(modbusDevice, &QModbusServer::stateChanged,
                this, &homewindow::onStateChanged);
        if(modbusDevice->state() != QModbusDevice::UnconnectedState)
            modbusDevice->disconnectDevice();
    }

    sigleGroupDialog::deleteInstance();
    s_Instance = Q_NULLPTR;
    delete ui;
    qDebug("homewindow exit");
}

void homewindow::on_spinBox_groupNum_valueChanged(int arg1)//Add or reduce group
{
    while(m_groups.count() < arg1){
        int _groupID = m_groups.count()+1;
        int _fanMaxNumber = gMotorMaxnum;
        int _startAdd = (_groupID-1)*_fanMaxNumber + 1;
        FanGroupInfo *group = new FanGroupInfo{_groupID, _startAdd, _fanMaxNumber};
        group->m_monitorDialog = Q_NULLPTR;
        m_groups.push_back(group);
    }
    while(m_groups.count() > arg1){
        m_groups.pop_back();
    }


    for(int i = arg1; i < gGroupnum; i++)//!< For reduce group
    {
        m_groupBtn.at(i)->setEnabled(false);
    }
    for(int i = 0; i < arg1; i++)        //!< For add group
    {
        m_groupBtn.at(i)->setEnabled(true);
    }
}

void homewindow::button_group_clicked()//All group buttons clicked (use one slot fuction)
{
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    for(int i = 0; i < gGroupnum; i++){
        if(btn == m_groupBtn.at(i)){
            mCurrentGroupInfo = m_groups.at(i);
            sigleGroupDialog::getS_Instance()->show(mCurrentGroupInfo);
            break;
        }
    }

}


QStatusBar* homewindow::statusBar()//Get mainwindow's status bar
{
    return MainWindow::getInstance()->getStatusBar();
}


QVector<FanGroupInfo *> *homewindow::groups()
{
    return &m_groups;
}

homewindow *homewindow::getInstance()
{
    if(!s_Instance)
    {
        s_Instance = new homewindow;
    }
    return s_Instance;
}

void homewindow::deleteInstance()
{
    if(s_Instance)
        s_Instance->deleteLater();
}

void homewindow::on_connectButton_clicked()
{
    bool intendToConnect = (modbusDevice->state() == QModbusDevice::UnconnectedState);

    statusBar()->clearMessage();

    if (intendToConnect) {

        const QUrl url = QUrl::fromUserInput(ui->portEdit->text());
        modbusDevice->setConnectionParameter(QModbusDevice::NetworkPortParameter, url.port());
        modbusDevice->setConnectionParameter(QModbusDevice::NetworkAddressParameter, url.host());

        modbusDevice->setServerAddress(ui->serverEdit->text().toInt());
        if (!modbusDevice->connectDevice()) {
            statusBar()->showMessage(tr("Connect failed: ") + modbusDevice->errorString(), 5000);
        } else {

        }
    } else {
        modbusDevice->disconnectDevice();

    }
}

/*!
 * \brief homewindow::onStateChanged
 * \param state
 */
void homewindow::onStateChanged(int state)
{
    if(state == QModbusDevice::ConnectedState){
        ui->connectButton->setText(tr("  Disconnect  "));
        statusBar()->showMessage(tr("Modbus Tcp server connected !"), 5000);
    }
    else if(state == QModbusDevice::ConnectingState){
        ui->connectButton->setText(tr("   Connect    "));
        statusBar()->showMessage(tr("Modbus Tcp server connecting ... !"), 15000);
    }
    else{
        ui->connectButton->setText(tr("   Connect    "));
        statusBar()->showMessage(tr("Modbus Tcp server disonnected !"), 5000);
    }
}

void homewindow::handleDeviceError(QModbusDevice::Error newError)
{
    if (newError == QModbusDevice::NoError || !modbusDevice)
        return;

    statusBar()->showMessage(modbusDevice->errorString(), 5000);
}

void homewindow::clientToConnect()
{
    //![1] Make sure modbus device was not created yet
    if (modbusTcpClient) {
        modbusTcpClient->disconnectDevice();
        delete modbusTcpClient;
        modbusTcpClient = Q_NULLPTR;
    }

    //![2] Create modbus tcp client device and monitor error,state
    modbusTcpClient = new QModbusTcpClient(this);

    connect(modbusTcpClient, &QModbusClient::errorOccurred, [this](QModbusDevice::Error) {
        statusBar()->showMessage(modbusTcpClient->errorString(), 3000);
    });

    if (!modbusTcpClient) {
        statusBar()->showMessage(tr("Could not create Modbus client."), 3000);
        return;
    }
qDebug()<<"creat client";
    statusBar()->clearMessage();//!< Clear status bar first, we will use it

    //![3] Set port parameter and connect device
    if (modbusTcpClient->state() != QModbusDevice::ConnectedState) {

        const QUrl url = QUrl::fromUserInput(ui->portEdit->text());
        modbusTcpClient->setConnectionParameter(QModbusDevice::NetworkPortParameter, 6475);
        modbusTcpClient->setConnectionParameter(QModbusDevice::NetworkAddressParameter, "172.27.35.1");

        modbusTcpClient->setTimeout(100);//!< Set response timeout 100ms
        modbusTcpClient->setNumberOfRetries(2);//!< Set retry number 2
qDebug()<<" client try to connect";
        //! Now try connect modbus device
        if (!modbusTcpClient->connectDevice()) {//!< Connect failed
qDebug()<<" client connect fail";
            statusBar()->showMessage(tr("Modbus connect failed: ") + modbusTcpClient->errorString(), 3000);
        } else {//!< Connect  successfully

        }
        qDebug()<<modbusTcpClient->state();
    }
}

/*!
 * \brief homewindow::writeToMotor  local tcp client send data to remote tcp server
 * \param motorAdd
 * \param registerAdd
 * \param count
 */
void homewindow::writeToMotor(quint16 motorAdd, quint16 registerAdd, quint16 count)
{
    if (!modbusTcpClient)
        return;

    //![1] Find motor
    QMotor *motor;
    foreach(FanGroupInfo *_group, m_groups){//!< Get corresponding motor
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
    //! 0:15 register address , 16:31 motor address
    int startAddress = (registerAdd & 0x00ff) | ((motorAdd << 8) & 0xff00);
    int numberOfEntries = count;
    QModbusDataUnit writeUnit = QModbusDataUnit(table, startAddress, numberOfEntries);
    for (uint i = 0; i < writeUnit.valueCount(); i++) {

        writeUnit.setValue(i, *buff++);
    }
qDebug()<<"Send write request to tcp server";
    //![3] Send write request to tcp server (adress:)
    int tcpServerAddress = 1;
    if (auto *reply = modbusTcpClient->sendWriteRequest(writeUnit, tcpServerAddress)) {
        if (!reply->isFinished()) {
            connect(reply, &QModbusReply::finished, this, [this, reply]() {
                if (reply->error() == QModbusDevice::ProtocolError) {
                    statusBar()->showMessage(tr("Write response error: %1 (Mobus exception: 0x%2)")
                                             .arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 16)
                                             );
                } else if (reply->error() != QModbusDevice::NoError) {
                    statusBar()->showMessage(tr("Write response error: %1 (code: 0x%2)").
                                             arg(reply->errorString()).arg(reply->error(), -1, 16));
                }
                reply->deleteLater();
            });
        } else {//!< Broadcast replies return immediately
            reply->deleteLater();
        }
    } else {
        statusBar()->showMessage(tr("Write error: ") + modbusTcpClient->errorString());
    }


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

QModbusResponse homewindow::processReadHoldingRegistersRequest(const QModbusRequest &rqst)
{
    return readBytes(rqst, QModbusDataUnit::HoldingRegisters);
}
QModbusResponse homewindow::readBytes(const QModbusPdu &request,
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
    foreach(FanGroupInfo *_group, m_groups){//!< Get corresponding motor
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

QModbusResponse homewindow::processWriteMultipleRegistersRequest(
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

    //!
    //! [1] remote Client 6474  -> local Server 6474
    //! [2] remote Server 6475 open
    //! [3] remote Client 6474 send command "m_connectToServer" to local Server 6474
    //! [4] local Server 6474 recevied, then local client 6475 connect to remote Server 6475
    //! [5] local Client 6475  -> remote Server 6475
    //!
    //! Write Command
    if(registerAdd == g_motorCommandAddress){
        FCommandRegister fcr;
        quint16 *buff = (quint16 *)&fcr;

        const QByteArray pduData = request.data().remove(0,5);
        QDataStream stream(pduData);
        quint16 tmp;
        for (int i = 0; i < numberOfRegisters; i++) {
            stream >> tmp;
            *buff++ = tmp;
        }

        if(fcr.m_command == m_connectToServer){
qDebug() <<"connect to tcp server";
            clientToConnect();
        }
        else if(fcr.m_command == m_readMotorRegister){
            quint16 _address = fcr.m_registerAdd;
            quint16 _motorAdd = (_address >> 8) & 0x00ff ;
            quint16 _registerAdd = (_address) & 0x00ff ;
            //! Let modbus master send read request to slave
            qDebug() <<"Read   entries";
            emit readMotorRegister(_motorAdd, _registerAdd, fcr.m_count);
        }

        return QModbusResponse(request.functionCode(), address, numberOfRegisters);
    }

    //! Write motor register
    if(motorAdd != 0){//! To one motor

        QMotor *motor;
        foreach(FanGroupInfo *_group, m_groups){//!< Get corresponding motor
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
        //! Write motor register
        if(registerAdd == g_mRealTimeRegisterAddress)
            buff = (quint16 *)&motor->m_motorController;
        //! Write motor register
        else if(registerAdd == g_mSettingsRegisterAddress)
            buff = (quint16 *)&motor->m_initSetttings;
        //! Write error
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
        //! Let modbus master send write request to slave
        emit writeMotorRegister(motorAdd, registerAdd, numberOfRegisters);
    }
    else{//! Broadcast

        quint16 * buff;
        const QByteArray pduData = request.data().remove(0,5);
        foreach(FanGroupInfo *group, m_groups){
            foreach(QMotor *motor, group->m_motors){


                if(registerAdd == g_mRealTimeRegisterAddress)
                    buff = (quint16 *)&motor->m_motorController;
                else if(registerAdd == g_mSettingsRegisterAddress)
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
        //! Let modbus master send write request to slave
        emit writeMotorRegister(motorAdd, registerAdd, numberOfRegisters);
    }

    return QModbusResponse(request.functionCode(), address, numberOfRegisters);
}

QModbusResponse homewindow::processWriteSingleRegisterRequest(const QModbusRequest &rqst)
{
    return writeSingle(rqst, QModbusDataUnit::HoldingRegisters);
}

QModbusResponse homewindow::writeSingle(const QModbusPdu &request,
                                                  QModbusDataUnit::RegisterType unitType)
{
    Q_UNUSED(unitType)
    CHECK_SIZE_EQUALS(request);
    quint16 address, value;
    request.decodeData(&address, &value);

    quint16 motorAdd = (address >> 8) & 0x00ff ;
    quint16 registerAdd = (address) & 0x00ff ;

    QMotor *motor;
    foreach(FanGroupInfo *_group, m_groups){//!< Get corresponding motor
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

QModbusResponse homewindow::processRequest(const QModbusPdu &request)
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


FModbusTcpServer::FModbusTcpServer(QObject *parent)
{
    Q_UNUSED(parent)

}

QModbusResponse FModbusTcpServer::processRequest(const QModbusPdu &request)
{
    return homewindow::getInstance()->processRequest(request);

}
