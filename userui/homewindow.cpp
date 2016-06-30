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
        mGroup.append(ui->pushButton_group1);
        mGroup.append(ui->pushButton_group2);
        mGroup.append(ui->pushButton_group3);
        mGroup.append(ui->pushButton_group4);
        mGroup.append(ui->pushButton_group5);
        mGroup.append(ui->pushButton_group6);
        mGroup.append(ui->pushButton_group7);
        mGroup.append(ui->pushButton_group8);
        mGroup.append(ui->pushButton_group9);

        on_spinBox_groupNum_valueChanged(ui->spinBox_groupNum->value());//Init group number
    }

    for(int i = 0; i < gGroupnum; i++){//All group buttons use one slot fuction
        connect(mGroup.at(i), SIGNAL(clicked(bool)), this, SLOT(button_group_clicked()));
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
    while(mGroups.count() < arg1){
        int _groupID = mGroups.count()+1;
        int _fanMaxNumber = gMotorMaxnum;
        int _startAdd = (_groupID-1)*_fanMaxNumber + 1;
        FanGroupInfo *group = new FanGroupInfo{_groupID, _startAdd, _fanMaxNumber};
        group->m_monitorDialog = Q_NULLPTR;
        mGroups.push_back(group);
    }
    while(mGroups.count() > arg1){
        mGroups.pop_back();
    }


    for(int i = arg1; i < gGroupnum; i++)//!< For reduce group
    {
        mGroup.at(i)->setEnabled(false);
    }
    for(int i = 0; i < arg1; i++)        //!< For add group
    {
        mGroup.at(i)->setEnabled(true);
    }
}

void homewindow::button_group_clicked()//All group buttons clicked (use one slot fuction)
{
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    for(int i = 0; i < gGroupnum; i++){
        if(btn == mGroup.at(i)){
            mCurrentGroupInfo = mGroups.at(i);
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
    return &mGroups;
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
    foreach(FanGroupInfo *_group, mGroups){//!< Get corresponding motor
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

    // byte count does not match number of data bytes following or register count
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
        foreach(FanGroupInfo *_group, mGroups){//!< Get corresponding motor
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
    }
    else{
        quint16 * buff;
        const QByteArray pduData = request.data().remove(0,5);
        foreach(FanGroupInfo *group, mGroups){
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

QModbusResponse homewindow::processWriteSingleRegisterRequest(const QModbusRequest &rqst)
{
    return writeSingle(rqst, QModbusDataUnit::HoldingRegisters);
}

QModbusResponse homewindow::writeSingle(const QModbusPdu &request,
                                                  QModbusDataUnit::RegisterType unitType)
{
    CHECK_SIZE_EQUALS(request);
    quint16 address, value;
    request.decodeData(&address, &value);

    quint16 motorAdd = (address >> 8) & 0x00ff ;
    quint16 registerAdd = (address) & 0x00ff ;

    QMotor *motor;
    foreach(FanGroupInfo *_group, mGroups){//!< Get corresponding motor
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

}

QModbusResponse FModbusTcpServer::processRequest(const QModbusPdu &request)
{
    return homewindow::getInstance()->processRequest(request);

}
