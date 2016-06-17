#include "homewindow.h"
#include "ui_homewindow.h"

#include <QModbusTcpClient>
#include <QModbusRtuSerialMaster>
#include <QPushButton>
#include <QtCore>
#include "mainwindow/mainwindow.h"
#include "userui/fanmotorui.h"
#include "dialogs/siglegroupdialog.h"

extern CO_Data master_Data;

const int gGroupnum = 9; //Number of all groups

homewindow *homewindow::s_Instance = Q_NULLPTR;

homewindow::homewindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::homewindow)
    ,modbusDevice(Q_NULLPTR)
{
    ui->setupUi(this);

    mCurrentGroupInfo.m_groupID = 1;


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


//    FanMotorUi *fanmotorui = FanMotorUi::getS_Instance();
//    connect(this, SIGNAL(connectStateChanged(int)), fanmotorui, SLOT(onConnectStateChanged(int)));


}

homewindow::~homewindow()
{
    delete ui;
}

void homewindow::on_spinBox_groupNum_valueChanged(int arg1)//Add or reduce group
{
    for(int i = arg1; i < gGroupnum; i++)//For reduce group
    {
        mGroup.at(i)->setEnabled(false);
    }
    for(int i = 0; i < arg1; i++)        //For add group
    {
        mGroup.at(i)->setEnabled(true);
    }
}

void homewindow::button_group_clicked()//All group buttons clicked (use one slot fuction)
{
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    for(int i = 0; i < gGroupnum; i++){
        if(btn == mGroup.at(i)){
            mCurrentGroupInfo.m_groupID = i + 1;
            sigleGroupDialog *mSigleGroupDialog = sigleGroupDialog::getS_Instance();
            mSigleGroupDialog->getTitleBar()->getTitleLabel()->setText(tr("Group%1").arg(i+1));
            mSigleGroupDialog->show();
            break;
        }
    }

}

void homewindow::on_connectButton_clicked()//Connect button clicked
{
    //Tcp connect
    if(ui->radioButton_tcp->isChecked()){

        m_communication = CommunicationMode::Tcp;
    }

    //CAN connect
    if(ui->radioButton_can->isChecked()){

    }

    //Modbus connect
    if(ui->radioButton_modbus->isChecked()){

        //[1]Read serialport settings in "/SuperConsole.ini" (update in "SerialPortSettingsDialog" class)
        QSettings settings(QDir::currentPath() + "/SuperConsole.ini", QSettings::IniFormat);
        settings.beginGroup("SerialPortSettingsDialog");
        mSerialPortSettings.name = settings.value("SerialPortname", "").toString();
        mSerialPortSettings.baudRate = static_cast<QSerialPort::BaudRate>(settings.value("SerialPortBaudRate", "9600").toInt());
        mSerialPortSettings.dataBits = static_cast<QSerialPort::DataBits>(settings.value("SerialPortDataBits", "8").toInt());
        mSerialPortSettings.parity = static_cast<QSerialPort::Parity>(settings.value("SerialPortParity", "None").toInt());
        mSerialPortSettings.stopBits = static_cast<QSerialPort::StopBits>(settings.value("SerialPortStopBits", "1").toInt());
        mSerialPortSettings.flowControl = static_cast<QSerialPort::FlowControl>(settings.value("SerialPortFlowControl", "None").toInt());
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
            ui->connectButton->setDisabled(true);
            statusBar()->showMessage(tr("Could not create Modbus client."), 3000);
            return;
        } else {
            connect(modbusDevice, &QModbusClient::stateChanged,
                    this, &homewindow::onStateChanged);//Monitor modbus device state
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

            modbusDevice->setTimeout(300);//Set response timeout 300ms
            modbusDevice->setNumberOfRetries(2);//Set retry number 2


            //Now try connect modbus device
            if (!modbusDevice->connectDevice()) {//Connect failed
                statusBar()->showMessage(tr("Modbus connect failed: ") + modbusDevice->errorString(), 3000);
            } else {//Connect  successfully
            }
        }
    }
}

void homewindow::onStateChanged(int state)//Monitor modbus device state
{
    //Modbus state change
    if(ui->radioButton_modbus->isChecked()){
        bool connected = (state != QModbusDevice::UnconnectedState);
        ui->connectButton->setEnabled(!connected);
        ui->disconnectButton->setEnabled(connected);

        if(connected){
            m_communication = CommunicationMode::Modbus;
            statusBar()->showMessage(tr("Modbus connected !"), 3000);
        }
        else{
            m_communication = CommunicationMode::Init;
            statusBar()->showMessage(tr("Modbus disonnected !"), 3000);
        }

        emit connectStateChanged(connected);//Send state to sigleGroupDialog;
    }

    //CAN state change
    if(ui->radioButton_can->isChecked()){
        bool connected = (state != 0);
        ui->connectButton->setEnabled(!connected);
        ui->disconnectButton->setEnabled(connected);

        if(connected){
            m_communication = CommunicationMode::CANbus;
            statusBar()->showMessage(tr("CAN connected !"), 3000);
        }
        else{
            m_communication = CommunicationMode::Init;
            statusBar()->showMessage(tr("CAN disonnected !"), 3000);
        }

        emit connectStateChanged(connected);//Send state to sigleGroupDialog;
    }

    //Tcp state change
    if(ui->radioButton_tcp->isChecked()){
        bool connected = (state != 0);
        ui->connectButton->setEnabled(!connected);
        ui->disconnectButton->setEnabled(connected);

        if(connected){
            m_communication = CommunicationMode::Tcp;
            statusBar()->showMessage(tr("CAN connected !"), 3000);
        }
        else{
            m_communication = CommunicationMode::Init;
            statusBar()->showMessage(tr("CAN disonnected !"), 3000);
        }

        emit connectStateChanged(connected);//Send state to sigleGroupDialog;
    }
}

void homewindow::on_disconnectButton_clicked()//Disconnect button clicked
{
    //Tcp disconnect
    if(m_communication == CommunicationMode::Tcp){

    }

    //CAN disconnect
    if(m_communication == CommunicationMode::CANbus){


    }

    //Modbus disconnect
    if(m_communication == CommunicationMode::Modbus){
        if(!modbusDevice)
            return;
        if(modbusDevice->state() == QModbusDevice::ConnectedState){
            modbusDevice->disconnectDevice();
        }
    }

}



void homewindow::on_settingsButton_clicked()//Settings button clicked
{
    //Show mainwindow then show settingDialog
    MainWindow::getInstance()->show();
    SettingDialog* settingDialog = new SettingDialog;
    settingDialog->exec();//exec: if we don't close dialog ,we can't operate other window
}

QStatusBar* homewindow::statusBar()//Get mainwindow's status bar
{
    return MainWindow::getInstance()->getStatusBar();
}

QModbusClient* homewindow::getModbusDevice()
{
    return modbusDevice;
}

FanGroupInfo homewindow::getFanGroupInfo()
{
    mCurrentGroupInfo.m_com = m_communication;
    mCurrentGroupInfo.m_fanNumber = 12;

    return mCurrentGroupInfo;

}

homewindow *homewindow::getInstance()
{
    if(!s_Instance)
    {
        s_Instance = new homewindow;
    }
    return s_Instance;
}

