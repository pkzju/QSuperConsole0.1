#include "fanhomeframe.h"
#include "ui_fanhomeframe.h"
#include <QButtonGroup>
#include <QTableWidget>
#include <QModbusDataUnit>
#include <QTimer>

#include "dialogs/settingdialog.h"
#include "mainwindow/mainwindow.h"

FanHomeFrame::FanHomeFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::FanHomeFrame)
  , btnGroup(new QButtonGroup)
  , fans(new QButtonGroup)
  , m_speedRefData(new QCPDataMap)
  , m_speedFbkData(new QCPDataMap)
  , m_idRefData(new QCPDataMap)
  , m_idFbkData(new QCPDataMap)
  , m_iqRefData(new QCPDataMap)
  , m_iqFbkData(new QCPDataMap)
{
    ui->setupUi(this);
    qDebug("FanHomeFrame init");


    ui->textBrowser->document ()->setMaximumBlockCount (50);//!< Set textBrowser max block count

    firstTable = ui->frame1->getTable();
    secondTable = ui->frame2->getTable();

    btnGroup->addButton(reinterpret_cast<QAbstractButton *>(ui->frame1->getGroupBtn()), 1);
    btnGroup->addButton(reinterpret_cast<QAbstractButton *>(ui->frame2->getGroupBtn()), 2);
    connect(btnGroup, SIGNAL(buttonClicked(int)), this, SLOT(enterGroup(int)));

    fans->addButton(reinterpret_cast<QAbstractButton *>(ui->pushButton_fan1), 0);
    fans->addButton(reinterpret_cast<QAbstractButton *>(ui->pushButton_fan2), 1);
    fans->addButton(reinterpret_cast<QAbstractButton *>(ui->pushButton_fan3), 2);
    fans->addButton(reinterpret_cast<QAbstractButton *>(ui->pushButton_fan4), 3);
    fans->addButton(reinterpret_cast<QAbstractButton *>(ui->pushButton_fan5), 4);
    fans->addButton(reinterpret_cast<QAbstractButton *>(ui->pushButton_fan6), 5);
    fans->addButton(reinterpret_cast<QAbstractButton *>(ui->pushButton_fan7), 6);
    fans->addButton(reinterpret_cast<QAbstractButton *>(ui->pushButton_fan8), 7);
    fans->addButton(reinterpret_cast<QAbstractButton *>(ui->pushButton_fan9), 8);
    fans->addButton(reinterpret_cast<QAbstractButton *>(ui->pushButton_fan10), 9);
    fans->addButton(reinterpret_cast<QAbstractButton *>(ui->pushButton_fan11), 10);
    fans->addButton(reinterpret_cast<QAbstractButton *>(ui->pushButton_fan12), 11);
    fans->addButton(reinterpret_cast<QAbstractButton *>(ui->pushButton_fan13), 12);
    fans->addButton(reinterpret_cast<QAbstractButton *>(ui->pushButton_fan14), 13);
    fans->addButton(reinterpret_cast<QAbstractButton *>(ui->pushButton_fan15), 14);
    connect(fans, SIGNAL(buttonClicked(int)), this, SLOT(changeMotor(int)));

    connect(ui->pushButton_startMotor, SIGNAL(clicked(bool)), this, SLOT(onRunStateButtonClicked()));
    connect(ui->pushButton_startMotor_A, SIGNAL(clicked(bool)), this, SLOT(onRunStateButtonClicked()));
    connect(ui->pushButton_startMotor_G, SIGNAL(clicked(bool)), this, SLOT(onRunStateButtonClicked()));
    connect(ui->pushButton_stopMotor, SIGNAL(clicked(bool)), this, SLOT(onRunStateButtonClicked()));
    connect(ui->pushButton_stopMotor_A, SIGNAL(clicked(bool)), this, SLOT(onRunStateButtonClicked()));
    connect(ui->pushButton_stopMotor_G, SIGNAL(clicked(bool)), this, SLOT(onRunStateButtonClicked()));

    connect(ui->initializeButton_2, SIGNAL(clicked(bool)), this, SLOT(on_initializeButton_clicked()));
    connect(ui->pushButton_startMotor_2, SIGNAL(clicked(bool)), ui->pushButton_startMotor, SIGNAL(clicked(bool)));
    connect(ui->pushButton_stopMotor_2, SIGNAL(clicked(bool)), ui->pushButton_stopMotor, SIGNAL(clicked(bool)));


    initUi();


}

void FanHomeFrame::initUi()
{
    ui->stackedWidget->setCurrentIndex(0);
    ui->spinBox_motorNum->setMaximum(gMotorMaxnum);

    setupRealtimeDataDemo(ui->customPlot);

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

    ui->comboBox_fanType_2->addItem(QStringLiteral("Type0"), 0);
    ui->comboBox_fanType_2->addItem(QStringLiteral("Type1"), 1);
    ui->comboBox_fanType_2->addItem(QStringLiteral("Type2"), 2);
    ui->comboBox_fanType_2->addItem(QStringLiteral("Type3"), 3);

    ui->comboBox_fanPressure_2->addItem(QStringLiteral("Pressure0"), 0);
    ui->comboBox_fanPressure_2->addItem(QStringLiteral("Pressure1"), 1);
    ui->comboBox_fanPressure_2->addItem(QStringLiteral("Pressure2"), 2);
    ui->comboBox_fanPressure_2->addItem(QStringLiteral("Pressure3"), 3);

    ui->comboBox_fanreGulation_2->addItem(QStringLiteral("Regulation0"), 0);
    ui->comboBox_fanreGulation_2->addItem(QStringLiteral("Regulation1"), 1);
    ui->comboBox_fanreGulation_2->addItem(QStringLiteral("Regulation2"), 2);
    ui->comboBox_fanreGulation_2->addItem(QStringLiteral("Regulation2"), 3);

    ui->comboBox_fanControlMode_2->addItem(QStringLiteral("Mode0"), 0);
    ui->comboBox_fanControlMode_2->addItem(QStringLiteral("Mode1"), 1);
}

FanHomeFrame::~FanHomeFrame()
{
    delete ui;
    qDebug("FanHomeFrame exit");
}

void FanHomeFrame::setGroup(QVector<FanGroupInfo*> *motorGroups,int start, int number)
{
    allGroups = motorGroups;

    groups.clear();
    for(int i = 0; i< number; i++)
    {
        groups.push_back(motorGroups->at(start+i));
    }

    FanGroupInfo *first = groups.at(0);
    FanGroupInfo *second = groups.at(1);

    ui->groupBox1->setTitle(tr("Group%1").arg(first->m_groupID));
    ui->groupBox2->setTitle(tr("Group%1").arg(second->m_groupID));

    {
        QVector<QMotor *> *_motors = &first->m_motors;
        for(int i=0; i < _motors->count(); i++){
            firstTable->setCellWidget(i, 0, _motors->at(i)->m_motorAddressLabel);
            firstTable->setCellWidget(i, 1, _motors->at(i)->m_targetPowerLCD);
            firstTable->setCellWidget(i, 2, _motors->at(i)->m_nowPowerLCD);
            firstTable->setCellWidget(i, 3, _motors->at(i)->m_speedRefLCD);
            firstTable->setCellWidget(i, 4, _motors->at(i)->m_speedFbkLCD);
            firstTable->setCellWidget(i, 5, _motors->at(i)->m_runLamp);
            firstTable->setCellWidget(i, 6, _motors->at(i)->m_commLamp);
            firstTable->setCellWidget(i, 7, _motors->at(i)->m_message);
        }
    }
    {
        QVector<QMotor *> *_motors = &second->m_motors;
        for(int i=0; i < _motors->count(); i++){
            secondTable->setCellWidget(i, 0, _motors->at(i)->m_motorAddressLabel);
            secondTable->setCellWidget(i, 1, _motors->at(i)->m_targetPowerLCD);
            secondTable->setCellWidget(i, 2, _motors->at(i)->m_nowPowerLCD);
            secondTable->setCellWidget(i, 3, _motors->at(i)->m_speedRefLCD);
            secondTable->setCellWidget(i, 4, _motors->at(i)->m_speedFbkLCD);
            secondTable->setCellWidget(i, 5, _motors->at(i)->m_runLamp);
            secondTable->setCellWidget(i, 6, _motors->at(i)->m_commLamp);
            secondTable->setCellWidget(i, 7, _motors->at(i)->m_message);
        }
    }

}

void FanHomeFrame::enterGroup(int id)
{
    ui->stackedWidget->setCurrentIndex(1);

    changeGroup(groups.at(id-1));

}

void FanHomeFrame::on_previous1_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void FanHomeFrame::on_next1_clicked()
{
    changeMotor(ui->spinBox_InitAdress->value()-currentGroup->m_startAddress);
}

void FanHomeFrame::on_previous2_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}


void FanHomeFrame::on_spinBox_motorNum_valueChanged(int arg1)
{
    if(arg1 > gMotorMaxnum)
        ui->spinBox_motorNum->setValue(gMotorMaxnum);

    if(!currentGroup)
        return;

    QVector<QMotor *> *m_motors = &currentGroup->m_motors;

    while(m_motors->count() < arg1){
        int __motorCount = m_motors->count();
        m_motors->push_back(new QMotor(__motorCount + currentGroup->m_startAddress));
    }

    while(m_motors->count() > arg1){
        m_motors->pop_back();
    }

    for(int i = arg1; i < 15; i++)//!< For reduce group
    {
        fans->button(i)->setEnabled(false);
    }
    for(int i = 0; i < arg1; i++)        //!< For add group
    {
        fans->button(i)->setEnabled(true);
    }

    if(currentGroup)//!< Update max address at current address
        ui->spinBox_InitAdress->setMaximum(currentGroup->m_startAddress + arg1 - 1);
}

void FanHomeFrame::on_settingsButton_clicked()
{
    SettingDialog* settingDialog = new SettingDialog();
    settingDialog->exec();//!< exec: if we don't close dialog ,we can't operate other window
}

void FanHomeFrame::on_clearButton_clicked()
{
    ui->textBrowser->clear();
}

void FanHomeFrame::changeGroup(FanGroupInfo *group)
{
    if(currentGroup == group)//!< If the same group, do nothing
        return;

    //! Get group parameter
    currentGroup = group; //!< Get group pointer
    QVector<QMotor *> *m_motors = &currentGroup->m_motors;//!< Get motor container pointer
    int _startAddress = currentGroup->m_startAddress; //!< Get current group start address
    int _fanMaxNumber = currentGroup->m_fanMaxNumber; //!< Get current group
    int _fanNumber = m_motors->count(); //!< Get current group motor number

    //! If current group motor number is 0, we reset it to maxNumber
    if(m_motors->count() == 0){
        _fanNumber = _fanMaxNumber;
        for(int i = 0; i< _fanNumber; i++){ //!< Init table
            m_motors->push_back(new QMotor(i+_startAddress));
        }
    }

    //! Update widget
    ui->lcdNumber_groupNum->display(currentGroup->m_groupID);
    ui->spinBox_motorNum->setValue(_fanNumber); //!< Motor number spinBox set the right number
    ui->spinBox_motorNum->setMaximum(_fanMaxNumber); //!< Motor number spinBox set maxNumber
    ui->spinBox_InitAdress->setMinimum(_startAddress); //!< SpinBox_InitAdress for first fan in current group
    ui->spinBox_InitAdress->setMaximum(_startAddress + _fanNumber - 1); //!< Last address in current group

    on_spinBox_motorNum_valueChanged(currentGroup->m_motors.count());
}

void FanHomeFrame::changeMotor(int id)
{
    currentMotor = currentGroup->m_motors.at(id);
    m_specialMotorAdd = currentMotor->m_address;
    ui->lcdNumber_fanID->display(m_specialMotorAdd);
    ui->checkBox->setChecked(true);
    ui->stackedWidget->setCurrentIndex(2);
    ui->spinBox_InitAdress->setValue(m_specialMotorAdd);

}

/* Init parameter set and read fuction part start *********************************/

void FanHomeFrame::on_pushButton_InitSetF_clicked()
{
    //![1] Init data
    QAbstractItemModel *__model = ui->table_settings->model();
    int _address = ui->spinBox_InitAdress->value();//Server address
    QMotor motor(0);
    quint16 *buff = (quint16 *)&motor.m_initSetttings;

    //![2] Get data from tableview
    for(unsigned char i = 0; i<3; i++){
        for(unsigned char j = 1; j<5; j++){
            if(j%2 == 0)
                *buff = (quint16)(__model->data(__model->index(i, j)).toInt());
            else
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
    int startAddress = g_mSettingsRegisterAddress;
    int numberOfEntries = g_mSettingsRegisterCount;
    QModbusDataUnit writeUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, startAddress, numberOfEntries);
    for (uint i = 0; i < writeUnit.valueCount(); i++) {

        writeUnit.setValue(i, *buff++);
    }

    //![4] Send write request to node(adress:)
    MainWindow::getInstance()->sendRtuWriteRequest(writeUnit, _address);
}

void FanHomeFrame::on_pushButton_InitSetG_clicked()
{
    //![1] Init data
    QAbstractItemModel *__model = ui->table_settings->model();
    int _startAddress = ui->spinBox_InitAdress->minimum();//!< Server address
    int _lastAddress = ui->spinBox_InitAdress->maximum();
    QMotor motor(0);
    quint16 *buff = (quint16 *)&motor.m_initSetttings;

    //![2] Get data from tableview
    for(unsigned char i = 0; i<3; i++){
        for(unsigned char j = 1; j<5; j++){
            if(j%2 == 0)
                *buff = (quint16)(__model->data(__model->index(i, j)).toInt());
            else
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
    int startAddress = g_mSettingsRegisterAddress;
    int numberOfEntries = g_mSettingsRegisterCount;
    QModbusDataUnit writeUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, startAddress, numberOfEntries);
    for (uint i = 0; i < writeUnit.valueCount(); i++) {

        writeUnit.setValue(i, *buff++);
    }

    //![4] Send write request to node(adress:i) at current group
    for(int i = _startAddress; i <= _lastAddress; i++){
        MainWindow::getInstance()->sendRtuWriteRequest(writeUnit, i);
    }

}

void FanHomeFrame::on_pushButton_InitSetA_clicked()
{
    //![1] Init data
    QAbstractItemModel *__model = ui->table_settings->model();
    int _address = 0;//!< Broadcast address
    QMotor motor(0);
    quint16 *buff = (quint16 *)&motor.m_initSetttings;

    //![2] Get data from tableview
    for(unsigned char i = 0; i<3; i++){
        for(unsigned char j = 1; j<5; j++){
            if(j%2 == 0)
                *buff = (quint16)(__model->data(__model->index(i, j)).toInt());
            else
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
    int startAddress = g_mSettingsRegisterAddress;
    int numberOfEntries = g_mSettingsRegisterCount;
    QModbusDataUnit writeUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, startAddress, numberOfEntries);
    for (uint i = 0; i < writeUnit.valueCount(); i++) {

        writeUnit.setValue(i, *buff++);
    }

    //![4] Send write request to node(adress:0) : Broadcast
    MainWindow::getInstance()->sendRtuWriteRequest(writeUnit, _address);

}

void FanHomeFrame::on_pushButton_InitRead_clicked()
{
    int startAddress = g_mSettingsRegisterAddress; //register address
    int numberOfEntries = g_mSettingsRegisterCount; //16bit register number
    qDebug() <<"Read number Of entries:"<<numberOfEntries;
    QModbusDataUnit _modbusDataUnit(QModbusDataUnit::HoldingRegisters, startAddress, numberOfEntries);

    if (auto *reply = MainWindow::getInstance()->sendRtuReadRequest(_modbusDataUnit, ui->spinBox_InitAdress->value())) {
        if (!reply->isFinished()){
            connect(reply, &QModbusReply::finished, this, &FanHomeFrame::readInitReady);
        }
        else
            delete reply; // broadcast replies return immediately
    }


}

void FanHomeFrame::readInitReady()
{
    QModbusReply *reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;

    readReplyHandle(reply);
}

/* Init parameter set and read fuction part end ***********************************/

/* Initialize fan / group / all motor fuction part start ******************************/

void FanHomeFrame::on_initializeButton_clicked()
{
    int startAddress = g_mControllerRegisterAddress; //!< register address
    int numberOfEntries = g_mRatedRegisterCount; //!< 16bit register number
    qDebug() <<"Read number Of entries:"<<numberOfEntries;
    QModbusDataUnit _modbusDataUnit(QModbusDataUnit::HoldingRegisters, startAddress, numberOfEntries);

    if (auto *reply = MainWindow::getInstance()->sendRtuReadRequest(_modbusDataUnit, ui->spinBox_InitAdress->value())) {
        if (!reply->isFinished()){
            connect(reply, &QModbusReply::finished, this, &FanHomeFrame::readInitFGAReady);
        }
        else
            delete reply; //!< Broadcast replies return immediately
    }
}

void FanHomeFrame::on_initializeGButton_clicked()
{
    //! Pack data unit
    int startAddress = g_mControllerRegisterAddress; //!< register address
    int numberOfEntries = g_mRatedRegisterCount; //!< 16bit register number
    QModbusDataUnit _modbusDataUnit(QModbusDataUnit::HoldingRegisters, startAddress, numberOfEntries);

    foreach(QMotor *_motor, currentGroup->m_motors){

        //! Send read request to one motor
        if (auto *reply = MainWindow::getInstance()->sendRtuReadRequest(_modbusDataUnit, _motor->m_address)) {
            if (!reply->isFinished()){
                connect(reply, &QModbusReply::finished, this, &FanHomeFrame::readInitFGAReady);
            }
            else
                delete reply; //!< Broadcast replies return immediately
        }
    }
}

void FanHomeFrame::on_initializeAButton_clicked()
{
    //! Pack data unit
    int startAddress = g_mControllerRegisterAddress; //register address
    int numberOfEntries = g_mRatedRegisterCount; //16bit register number
    QModbusDataUnit _modbusDataUnit(QModbusDataUnit::HoldingRegisters, startAddress, numberOfEntries);

    foreach(FanGroupInfo *_group, *allGroups){
        foreach(QMotor *_motor, _group->m_motors){
            //! Send read request to one motor
            if (auto *reply = MainWindow::getInstance()->sendRtuReadRequest(_modbusDataUnit, _motor->m_address)) {
                if (!reply->isFinished()){
                    connect(reply, &QModbusReply::finished, this, &FanHomeFrame::readInitFGAReady);
                }
                else
                    delete reply; //!< Broadcast replies return immediately
            }
        }
    }
}

void FanHomeFrame::readInitFGAReady()
{
    QModbusReply *reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;

    readReplyHandle(reply);

}

/* Initialize fan / group / all motor fuction part end *********************************/


void FanHomeFrame::onRunStateButtonClicked()
{
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    if(btn == ui->pushButton_startMotor || btn == ui->pushButton_stopMotor){

        //![1] Init data
        QMotor motor(0);
        int _address = ui->spinBox_InitAdress->value();//!< Server address
        if(btn == ui->pushButton_startMotor)
            motor.m_motorController.m_runState = FanMotorState::m_run;
        else
            motor.m_motorController.m_runState = FanMotorState::m_stop;
        quint16 *buff = (quint16 *)&motor.m_motorController;
        buff += 2;

        //![2] Pack data to a QModbusDataUnit
        int startAddress = g_mRealTimeRegisterAddress;
        int numberOfEntries = g_mRealTimeRegisterStateCount;
        QModbusDataUnit writeUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, startAddress, numberOfEntries);
        for (uint i = 0; i < writeUnit.valueCount(); i++) {

            writeUnit.setValue(i, *buff++);
        }
        //![3] Send write request to server (adress:)
        MainWindow::getInstance()->sendRtuWriteRequest(writeUnit, _address);

    }
    else if(btn == ui->pushButton_startMotor_G || btn == ui->pushButton_stopMotor_G){

        //![1] Init data
        QMotor motor(0);
        int _startAddress = ui->spinBox_InitAdress->minimum();//!<Server address
        int _lastAddress = ui->spinBox_InitAdress->maximum();
        if(btn == ui->pushButton_startMotor_G)
            motor.m_motorController.m_runState = FanMotorState::m_run;
        else
            motor.m_motorController.m_runState = FanMotorState::m_stop;
        quint16 *buff = (quint16 *)&motor.m_motorController;
        buff += 2;

        //![2] Pack data to a QModbusDataUnit
        int startAddress = g_mRealTimeRegisterAddress;
        int numberOfEntries = g_mRealTimeRegisterStateCount;
        QModbusDataUnit writeUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, startAddress, numberOfEntries);
        for (uint i = 0; i < writeUnit.valueCount(); i++) {

            writeUnit.setValue(i, *buff++);
        }
        qDebug() <<writeUnit.valueCount();

        //![3] Send write request to node(adress:i) at current group
        for(int i = _startAddress; i <= _lastAddress; i++){

            MainWindow::getInstance()->sendRtuWriteRequest(writeUnit, i);
        }

    }
    else if(btn == ui->pushButton_startMotor_A || btn == ui->pushButton_stopMotor_A){

        //![1] Init data
        int _address = 0;//!Broadcast address
        QMotor motor(0);
        if(btn == ui->pushButton_startMotor_A)
            motor.m_motorController.m_runState = FanMotorState::m_run;
        else
            motor.m_motorController.m_runState = FanMotorState::m_stop;
        quint16 *buff = (quint16 *)&motor.m_motorController;
        buff += 2;

        //![2] Pack data to a QModbusDataUnit
        int startAddress = g_mRealTimeRegisterAddress;
        int numberOfEntries = g_mRealTimeRegisterStateCount;
        QModbusDataUnit writeUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, startAddress, numberOfEntries);
        for (uint i = 0; i < writeUnit.valueCount(); i++) {

            writeUnit.setValue(i, *buff++);
        }

        MainWindow::getInstance()->sendRtuWriteRequest(writeUnit, _address);


    }
}

/* Group and all motor monitor fuction part start *********************************/

void FanHomeFrame::on_checkBox_monitorASW_stateChanged(int arg1)
{
   //![3.1] For modbus rtu mode, turn to fuction "monitor_stateChanged"
   monitor_stateChanged(arg1);
}

void FanHomeFrame::monitor_stateChanged(int state)
{
    //![1] If state not change, do nothing
    if(m_monitorState == (bool)state)
        return;

    //![2] Update state to variable "m_monitorState"
    m_monitorState = state;//!< Update monitor state

    //![3.1] If state > 0 , start  monitor, open timer
    if(state){
        QTimer::singleShot(0, [this]() { monitorTimer_update(); });
    }
}

void FanHomeFrame::monitorSigleStateChange(int state)
{
    int motorAddress = ui->spinBox_InitAdress->value();

    qDebug()<< motorAddress <<"isSigleMotorMonitor:"<<state;

    if(state){//!< Open state

        //! Load "m_specialMotorAdd" with current motor address
        m_specialMotorAdd = motorAddress;

        //! If not start yet, now start
        if(!m_monitorState){
            QTimer::singleShot(0, [this]() { monitorTimer_update(); });
        }
    }
    else//!< Close state
        m_specialMotorAdd = 0;//!< Load "m_specialMotorAdd" with 0, because no motor have address 0

}

/*!
 * \brief Monitor group and all timer update
 */
void FanHomeFrame::monitorTimer_update()
{
    //![1] Wait until no timerout response
    if(m_monitorTimeroutCount > 0){

        //! Delay some time , in case one cycle not complete yet
        QTimer::singleShot(m_monitorTimeroutCount*(g_responseTimeout+100)*g_numberOfTry, [this]() { monitorTimer_update(); });
        m_monitorTimeroutCount = 0;
        return;
    }


    //![3] Init variable
    m_monitorTimerPeriod = 0; //!< For count next monitor period time
    m_monitorTimeroutCount = 0; //!< For add next monitor period time when timeout happen
    int startAddress = g_mRealTimeRegisterAddress; //!< Register start address
    int numberOfEntries = g_mRealTimeRegisterCount; //!< 16bit register number
    //![3.1] Pack data unit
    QModbusDataUnit modbusDataUnit(QModbusDataUnit::HoldingRegisters, startAddress, numberOfEntries);

    //![4] For each motor, check state and send request
    foreach(FanGroupInfo *_group, *allGroups){
        foreach(QMotor *_motor, _group->m_motors){

            //![4.1] If is the special motor, we need more information, change the number of entries
            if(_motor->m_address == m_specialMotorAdd){

                modbusDataUnit.setValueCount(g_mRealTimeRegisterMoreCount);//!< Change the number of entries
                //! Send reqeust to special motor
                if (auto *reply =  MainWindow::getInstance()->sendRtuReadRequest(modbusDataUnit, m_specialMotorAdd)) {
                    if (!reply->isFinished()){
                        connect(reply, &QModbusReply::finished, this, &FanHomeFrame::monitorReadReady);
                    }
                    else
                        delete reply; //!< Broadcast replies return immediately
                }
                m_monitorTimerPeriod += g_responseTimeout;//!< One motor request ,then period value add
            }
            //![4.2] If not the special motor and monitoring has begun, check motor monitor state
            else if(m_monitorState && _motor->isMonitor){

                modbusDataUnit.setValueCount(g_mRealTimeRegisterCount);//!< Change the number of entries
                //! Send reqeust to the not special motor
                if (auto *reply = MainWindow::getInstance()->sendRtuReadRequest(modbusDataUnit, _motor->m_address)) {
                    if (!reply->isFinished()){
                        connect(reply, &QModbusReply::finished, this, &FanHomeFrame::monitorReadReady);
                    }
                    else
                        delete reply; //!< Broadcast replies return immediately
                }

                m_monitorTimerPeriod += g_responseTimeout;//!< One motor request ,then period value add

                //! For special motor , we need real time data, so we send reqeust after each other motor reqeust
                if(m_specialMotorAdd > 0){

                    modbusDataUnit.setValueCount(g_mRealTimeRegisterMoreCount);//!< Change the number of entries
                    //! Send reqeust to special motor
                    if (auto *reply = MainWindow::getInstance()->sendRtuReadRequest(modbusDataUnit, m_specialMotorAdd)) {
                        if (!reply->isFinished()){
                            connect(reply, &QModbusReply::finished, this, &FanHomeFrame::monitorReadReady);
                        }
                        else
                            delete reply; //!< Broadcast replies return immediately
                    }
                    m_monitorTimerPeriod += g_responseTimeout;//!< One motor request ,then period value add
                }
            }
        }
    }

    //![5] IF not interrupted ,go on next period
    if(m_monitorState || (m_specialMotorAdd != 0)){

        if(m_monitorTimerPeriod < 100)
            m_monitorTimerPeriod = 100;
        QTimer::singleShot(m_monitorTimerPeriod*g_numberOfTry, [this]() { monitorTimer_update(); });
    }

}

/*!
 * \brief Monitor group and all read ready
 */
void FanHomeFrame::monitorReadReady()
{
    QModbusReply *reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;

    if(!readReplyHandle(reply)){
        m_monitorTimeroutCount++;
    }
}

/* Group and all motor monitor fuction part end ************************************/

void FanHomeFrame::updateMotorUi(int arg1)
{
    if(!currentMotor)
        return;
    //! Init parameter update
    if(arg1 == 0){
        //Update data to settings table
        QAbstractItemModel *__model = ui->table_settings_2->model();
        quint16 *__buffPtr = (quint16 *)&currentMotor->m_initSetttings;
        for(unsigned char i = 0; i<3; i++){
            for(unsigned char j = 1; j<5; j++){
                if(j%2 == 0)
                    __model->setData(__model->index(i,j), (int)(*__buffPtr));
                else
                    __model->setData(__model->index(i,j), (double)(*__buffPtr)  * 0.01);
                __buffPtr++;
            }
        }

        int _fanType = (*__buffPtr >> 12) & 0x000f;
        ui->comboBox_fanType_2->setCurrentIndex(_fanType);
        int _fanPressure = (*__buffPtr >> 10) & 0x0003;
        ui->comboBox_fanPressure_2->setCurrentIndex(_fanPressure);
        int _fanreGulation = (*__buffPtr >> 8) & 0x0003;
        ui->comboBox_fanreGulation_2->setCurrentIndex(_fanreGulation);
        int _fanControlMode = (*__buffPtr >> 7) & 0x0001;
        ui->comboBox_fanControlMode_2->setCurrentIndex(_fanControlMode);
    }

    //! Monitor data update
    if(arg1 == 1){
        ui->lcdNumber_targetPower->display(currentMotor->m_motorController.m_targetpower*0.01);
        ui->lcdNumber_realTimePower->display(currentMotor->m_motorController.m_nowpower*0.01);

        ui->lcdNumber_targetSpeed->display(currentMotor->m_motorController.m_speedRef);
        ui->lcdNumber_realTimeSpeed->display(currentMotor->m_motorController.m_speedFbk);

        ui->lcdNumber_targetId->display(currentMotor->m_motorController.m_idRef);
        ui->lcdNumber_realTimeId->display(currentMotor->m_motorController.m_idFbk);

        ui->lcdNumber_targetIq->display(currentMotor->m_motorController.m_iqRef);
        ui->lcdNumber_realTimeIq->display(currentMotor->m_motorController.m_iqFbk);


        double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
        m_speedRefData->insertMulti(key, QCPData(key, currentMotor->m_motorController.m_speedRef));
        m_speedFbkData->insertMulti(key, QCPData(key, currentMotor->m_motorController.m_speedFbk));
        m_idRefData->insertMulti(key, QCPData(key, currentMotor->m_motorController.m_idRef));
        m_idFbkData->insertMulti(key, QCPData(key, currentMotor->m_motorController.m_idFbk));
        m_iqRefData->insertMulti(key, QCPData(key, currentMotor->m_motorController.m_iqRef));
        m_iqFbkData->insertMulti(key, QCPData(key, currentMotor->m_motorController.m_iqFbk));

        //! remove data of lines that's outside visible range:
        if(m_speedRefData->count() > 100){
            m_speedRefData->remove(m_speedRefData->firstKey());
            m_speedFbkData->remove(m_speedFbkData->firstKey());
            m_idRefData->remove(m_idRefData->firstKey());
            m_idFbkData->remove(m_idFbkData->firstKey());
            m_iqRefData->remove(m_iqRefData->firstKey());
            m_iqFbkData->remove(m_iqFbkData->firstKey());
        }

        if(ui->checkBox->isChecked())
            realtimeDataSlot();

    }

    //! Initialize data update
    if(arg1 == 2){
        ui->lcdNumber_ratedPower->display(currentMotor->m_motorController.m_ratedPower);
        ui->lcdNumber_ratedSpeed->display(currentMotor->m_motorController.m_ratedSpeed);
    }

    if(currentMotor->m_motorController.m_runState == FanMotorState::m_run)
        ui->runLamp->setLampState(QcwIndicatorLamp::lamp_green);
    else if(currentMotor->m_motorController.m_runState == FanMotorState::m_stop)
        ui->runLamp->setLampState(QcwIndicatorLamp::lamp_yellow);
    else{

        ui->runLamp->setLampState(QcwIndicatorLamp::lamp_red);

        if(currentMotor->m_motorController.m_runState == FanMotorState::m_stall){
            ui->lineEdit_runState->setText(QObject::tr("stall"));
        }
        else if(currentMotor->m_motorController.m_runState == FanMotorState::m_overSpd){
            ui->lineEdit_runState->setText(QObject::tr("over speed"));
        }
        else if(currentMotor->m_motorController.m_runState == FanMotorState::m_error){
            if(currentMotor->m_motorController.m_runError == FanMotorError::m_overCur){
                ui->lineEdit_runState->setText(QObject::tr("over current"));
            }
            else if(currentMotor->m_motorController.m_runError == FanMotorError::m_overSpd_){
                ui->lineEdit_runState->setText(QObject::tr("over speed"));
            }
            else if(currentMotor->m_motorController.m_runError == FanMotorError::m_stall_){
                ui->lineEdit_runState->setText(QObject::tr("stall"));
            }
            else if(currentMotor->m_motorController.m_runError == FanMotorError::m_lowVolt){
                ui->lineEdit_runState->setText(QObject::tr("low voltage "));
            }
            else if(currentMotor->m_motorController.m_runError == FanMotorError::m_highVolt){
                ui->lineEdit_runState->setText(QObject::tr("high voltage "));
            }
            else
                ui->lineEdit_runState->setText(QObject::tr("no error"));
        }
    }


    if(currentMotor->m_communicationState == FanCommunicationState::m_connect)
        ui->comLamp->setLampState(QcwIndicatorLamp::lamp_green);
    else if(currentMotor->m_communicationState == FanCommunicationState::m_disconnect)
        ui->comLamp->setLampState(QcwIndicatorLamp::lamp_red);
    else if(currentMotor->m_communicationState == FanCommunicationState::m_comError)
        ui->comLamp->setLampState(QcwIndicatorLamp::lamp_yellow);
    else
        ui->comLamp->setLampState(QcwIndicatorLamp::lamp_grey);

    if(arg1 == 3){//Read PI ready
        ui->SpinBox_speedPRW->setValue((double) currentMotor->m_PIPara.m_speedKp * 0.0001);
        ui->SpinBox_speedIRW->setValue((double) currentMotor->m_PIPara.m_speedKi * 0.0001);
        ui->SpinBox_idPRW->setValue((double) currentMotor->m_PIPara.m_idKp * 0.0001);
        ui->SpinBox_idIRW->setValue((double) currentMotor->m_PIPara.m_idKi * 0.0001);
        ui->SpinBox_iqPRW->setValue((double) currentMotor->m_PIPara.m_iqKp * 0.0001);
        ui->SpinBox_iqIRW->setValue((double) currentMotor->m_PIPara.m_iqKi * 0.0001);
    }

}

void FanHomeFrame::setupRealtimeDataDemo(QCustomPlot *customPlot)
{
    // include this section to fully disable antialiasing for higher performance:
    customPlot->setNotAntialiasedElements(QCP::aeAll);
    QFont font;
    font.setStyleStrategy(QFont::NoAntialias);
    customPlot->xAxis->setTickLabelFont(font);
    customPlot->yAxis->setTickLabelFont(font);
    customPlot->legend->setFont(font);


    customPlot->addGraph(); // blue line
    customPlot->graph(0)->setPen(QPen(Qt::blue));
    customPlot->graph(0)->setName("Set");
    customPlot->addGraph(); // red line
    customPlot->graph(1)->setPen(QPen(Qt::red));
    customPlot->graph(1)->setName("Feedback");
//    customPlot->addGraph(); // green line
//    customPlot->graph(2)->setPen(QPen(Qt::green));


    customPlot->addGraph(); // blue dot
    customPlot->graph(2)->setPen(QPen(Qt::blue));
    customPlot->graph(2)->setLineStyle(QCPGraph::lsNone);
    customPlot->graph(2)->setScatterStyle(QCPScatterStyle::ssDisc);
    customPlot->addGraph(); // red dot
    customPlot->graph(3)->setPen(QPen(Qt::red));
    customPlot->graph(3)->setLineStyle(QCPGraph::lsNone);
    customPlot->graph(3)->setScatterStyle(QCPScatterStyle::ssDisc);


    customPlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    customPlot->xAxis->setDateTimeFormat("ss");
    customPlot->xAxis->setAutoTickStep(false);
    customPlot->xAxis->setTickStep(1);
    //  customPlot->axisRect()->setupFullAxesBox();

    // setup legend:
    customPlot->legend->setVisible(true);
    customPlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop|Qt::AlignLeft);
    customPlot->legend->setBrush(QColor(80, 80, 80, 200));
    QPen legendPen;
    legendPen.setColor(QColor(130, 130, 130, 200));
    customPlot->legend->setBorderPen(legendPen);
    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    customPlot->legend->setTextColor(Qt::white);
    customPlot->legend->removeItem(customPlot->legend->itemCount()-1);
    customPlot->legend->removeItem(customPlot->legend->itemCount()-1);

    // set some pens, brushes and backgrounds:
    customPlot->xAxis->setBasePen(QPen(Qt::white, 1));
    customPlot->yAxis->setBasePen(QPen(Qt::white, 1));
    customPlot->xAxis->setTickPen(QPen(Qt::white, 1));
    customPlot->yAxis->setTickPen(QPen(Qt::white, 1));
    customPlot->xAxis->setSubTickPen(QPen(Qt::white, 1));
    customPlot->yAxis->setSubTickPen(QPen(Qt::white, 1));
    customPlot->xAxis->setTickLabelColor(Qt::white);
    customPlot->yAxis->setTickLabelColor(Qt::white);
    customPlot->xAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
    customPlot->yAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
    customPlot->xAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
    customPlot->yAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
    customPlot->xAxis->grid()->setSubGridVisible(true);
    customPlot->yAxis->grid()->setSubGridVisible(true);
    customPlot->xAxis->grid()->setZeroLinePen(Qt::NoPen);
    customPlot->yAxis->grid()->setZeroLinePen(Qt::NoPen);
    customPlot->xAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);
    customPlot->yAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);
    QLinearGradient plotGradient;
    plotGradient.setStart(0, 0);
    plotGradient.setFinalStop(0, 350);
    plotGradient.setColorAt(0, QColor(80, 80, 80));
    plotGradient.setColorAt(1, QColor(50, 50, 50));
    customPlot->setBackground(plotGradient);
    QLinearGradient axisRectGradient;
    axisRectGradient.setStart(0, 0);
    axisRectGradient.setFinalStop(0, 350);
    axisRectGradient.setColorAt(0, QColor(80, 80, 80));
    axisRectGradient.setColorAt(1, QColor(30, 30, 30));
    customPlot->axisRect()->setBackground(axisRectGradient);

    for(int i=0; i <100; i++){
        double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;

        m_speedRefData->insertMulti(key, QCPData(key, 0));
        m_speedFbkData->insertMulti(key, QCPData(key, 0));
        m_idRefData->insertMulti(key, QCPData(key, 0));
        m_idFbkData->insertMulti(key, QCPData(key, 0));
        m_iqRefData->insertMulti(key, QCPData(key, 0));
        m_iqFbkData->insertMulti(key, QCPData(key, 0));

    }

    ui->customPlot->graph(0)->setData(m_speedRefData, true);
    ui->customPlot->graph(1)->setData(m_speedFbkData, true);

    ui->customPlot->xAxis->setRange(ui->customPlot->graph(0)->data()->firstKey(), ui->customPlot->graph(0)->data()->lastKey()+0.1);
    // make left and bottom axes transfer their ranges to right and top axes:
    connect(customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->yAxis2, SLOT(setRange(QCPRange)));

}

void FanHomeFrame::realtimeDataSlot()
{

    double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;


    //! set data :
    ui->customPlot->graph(2)->clearData();
    ui->customPlot->graph(3)->clearData();

    if(ui->radioButton_speed->isChecked()){
        ui->customPlot->graph(0)->setData(m_speedRefData, true);
        ui->customPlot->graph(1)->setData(m_speedFbkData, true);
        ui->customPlot->graph(2)->addData(key, currentMotor->m_motorController.m_speedRef);
        ui->customPlot->graph(3)->addData(key, currentMotor->m_motorController.m_speedFbk);
    }
    else if(ui->radioButton_id->isChecked()){
        ui->customPlot->graph(0)->setData(m_idRefData, true);
        ui->customPlot->graph(1)->setData(m_idFbkData, true);
        ui->customPlot->graph(2)->addData(key, currentMotor->m_motorController.m_idRef);
        ui->customPlot->graph(3)->addData(key, currentMotor->m_motorController.m_idFbk);
    }
    else if(ui->radioButton_iq->isChecked()){
        ui->customPlot->graph(0)->setData(m_iqRefData, true);
        ui->customPlot->graph(1)->setData(m_iqFbkData, true);
        ui->customPlot->graph(2)->addData(key, currentMotor->m_motorController.m_iqRef);
        ui->customPlot->graph(3)->addData(key, currentMotor->m_motorController.m_iqFbk);
    }

    //! rescale value (vertical) axis to fit the current data:
    ui->customPlot->graph(0)->rescaleValueAxis(false);
    ui->customPlot->graph(1)->rescaleValueAxis(true);

    ui->customPlot->xAxis->setRange(ui->customPlot->graph(0)->data()->firstKey(), \
                                    ui->customPlot->graph(0)->data()->lastKey()+0.1);

    ui->customPlot->replot();

}


bool FanHomeFrame::readReplyHandle(QModbusReply *reply)
{
    QMotor *motor = findMotor(reply->serverAddress());

    const QModbusDataUnit unit = reply->result();//!< Get reply data unit

    quint16 startAddress = unit.startAddress();
    quint16 valueCount = unit.valueCount();

    if(!motor)
        return false;

    //![1.1] Reply no error
    if (reply->error() == QModbusDevice::NoError) {

        //![][1] Check which registers
        quint16 * buff = findRegister(motor, (startAddress & 0x00ff));
        if(!buff){
            reply->deleteLater();
            return false;
        }

        //![][2] Update data to motor
        for (uint i = 0; i < valueCount; i++) {

            *buff++ = unit.value(i);

            const QString entry = tr("Address: %1, Value: %2").arg(QString::number(unit.startAddress()+i, 16))
                    .arg(QString::number(unit.value(i), 16));
            ui->textBrowser->append(entry);//!< Show out the data
        }

        //![][3] update Settings data to ui
        if((startAddress & 0x00ff)== g_mSettingsRegisterAddress && \
                motor->m_address == ui->spinBox_InitAdress->value()){

            //! Update data to settings view
            QAbstractItemModel *__model = ui->table_settings->model();
            quint16 *__buffPtr = (quint16 *)&motor->m_initSetttings;
            for(unsigned char i = 0; i<3; i++){
                for(unsigned char j = 1; j<5; j++){
                    if(j%2 == 0)
                        __model->setData(__model->index(i,j), (int)(*__buffPtr));
                    else
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
            updateMotorUi(0);
        }
        //! For update sigle motor ui
        else if((startAddress & 0x00ff) == g_mRealTimeRegisterAddress && motor->m_address == m_specialMotorAdd){
            updateMotorUi(1);
        }
        else if((startAddress & 0x00ff) == g_mControllerRegisterAddress && motor->m_address == m_specialMotorAdd){
            updateMotorUi(2);
        }
        else if((startAddress & 0x00ff) == g_mPIParaRegisterAddress && motor->m_address == m_specialMotorAdd){
            updateMotorUi(3);
        }

        motor->m_communicationState =FanCommunicationState::m_connect;//!< Update communication state

//        //![][4] Send to remote server
//        if(m_communication == CommunicationMode::Modbus){
//            emit writeMotorRegister(motor->m_address, startAddress, valueCount);
//        }

    }
    //![1.2] Reply protocol error
    else if (reply->error() == QModbusDevice::ProtocolError) {
        ui->textBrowser->append(tr("Read response error: %1 (Mobus exception: 0x%2)").
                                arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 16));
        motor->m_communicationState =FanCommunicationState::m_comError;//Update communication state
    }
    //![1.3] Reply timeout
    else {
        ui->textBrowser->append(tr("Read response error: %1 (code: 0x%2)").
                                arg(reply->errorString()).arg(reply->error(), -1, 16));
        motor->m_communicationState =FanCommunicationState::m_disconnect;//!< Update communication state

    }

    //![2] Update motor data to ui
    motor->update();

//    //![3] Send communication state to remote server
//    if(m_communication == CommunicationMode::Modbus){

//        emit writeMotorRegister(motor->m_address, g_mComStateAddress, g_mComStateRegisterCount);
//    }

    if(reply->error() == QModbusDevice::NoError){
        reply->deleteLater();
        return true;
    }
    else{
        reply->deleteLater();
        return false;
    }
}

QMotor *FanHomeFrame::findMotor(quint16 address)
{
    foreach(FanGroupInfo *_group, *allGroups){//!< Get corresponding motor
        //! Judge belong to which group
        if(address >= _group->m_startAddress && \
                address < _group->m_startAddress + _group->m_motors.count())
        {
            return _group->m_motors.at(address - _group->m_startAddress);
        }
    }

    return Q_NULLPTR;
}

quint16 *FanHomeFrame::findRegister(QMotor *motor, quint16 registerAddress)
{
    quint16 *buff = Q_NULLPTR;

    if((registerAddress & 0x00ff)== g_mSettingsRegisterAddress){//!< Make sure is Init data
        buff = (quint16 *)&motor->m_initSetttings;//!< Get current motor's initSetttings pointer
    }
    else if((registerAddress & 0x00ff) == g_mRealTimeRegisterAddress){
        buff = (quint16 *)&motor->m_motorController;
        buff += 2;
    }
    else if((registerAddress & 0x00ff) == g_mControllerRegisterAddress){
        buff = (quint16 *)&motor->m_motorController;
    }
    else if((registerAddress & 0x00ff) == g_mPIParaRegisterAddress){//!< Make sure is PI data
        buff = (quint16 *)&motor->m_PIPara;
    }
    else if((registerAddress & 0x00ff) == g_mComStateAddress){
        buff = (quint16 *)&motor->m_communicationState;
    }
    else{
        buff = Q_NULLPTR;
    }

    return buff;
}

void FanHomeFrame::on_checkBox_stateChanged(int arg1)
{
    monitorSigleStateChange(arg1);
}


void FanHomeFrame::on_pushButton_InitRead_2_clicked()
{
    on_pushButton_InitRead_clicked();
}

void FanHomeFrame::on_pushButton_InitSetF_2_clicked()
{
    //![1] Init data
    QAbstractItemModel *__model = ui->table_settings_2->model();
    int _address = ui->spinBox_InitAdress->value();//Server address
    QMotor motor(0);
    quint16 *buff = (quint16 *)&motor.m_initSetttings;

    //![2] Get data from tableview
    for(unsigned char i = 0; i<3; i++){
        for(unsigned char j = 1; j<5; j++){
            if(j%2 == 0)
                *buff = (quint16)(__model->data(__model->index(i, j)).toInt());
            else
                *buff = (quint16)(__model->data(__model->index(i, j)).toDouble() * 100);
            buff++;
        }
    }
    int _fanType = ui->comboBox_fanType_2->currentIndex();
    int _fanPressure = ui->comboBox_fanPressure_2->currentIndex();
    int _fanreGulation = ui->comboBox_fanreGulation_2->currentIndex();
    int _fanControlMode = ui->comboBox_fanControlMode_2->currentIndex();
    *buff = (_fanType << 12) + (_fanPressure << 10) + (_fanreGulation << 8) + (_fanControlMode << 7);

    //![3] Pack data to a QModbusDataUnit
    buff = (quint16 *)&motor.m_initSetttings;
    int startAddress = g_mSettingsRegisterAddress;
    int numberOfEntries = g_mSettingsRegisterCount;
    QModbusDataUnit writeUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, startAddress, numberOfEntries);
    for (uint i = 0; i < writeUnit.valueCount(); i++) {

        writeUnit.setValue(i, *buff++);
    }

    //![4] Send write request to node(adress:)
    MainWindow::getInstance()->sendRtuWriteRequest(writeUnit, _address);
}

void FanHomeFrame::on_readPIBtn_clicked()
{
    int startAddress = g_mPIParaRegisterAddress; //!< register address
    int numberOfEntries = g_mPIParaRegisterCount; //!< 16bit register number
    qDebug() <<"Read number Of entries:"<<numberOfEntries;
    QModbusDataUnit _modbusDataUnit(QModbusDataUnit::HoldingRegisters, startAddress, numberOfEntries);

    if (auto *reply = MainWindow::getInstance()->sendRtuReadRequest(_modbusDataUnit, ui->spinBox_InitAdress->value())) {
        if (!reply->isFinished()){
            connect(reply, &QModbusReply::finished, this, &FanHomeFrame::readPIReady);
        }
        else
            delete reply; //!< Broadcast replies return immediately
    }
}

void FanHomeFrame::readPIReady()
{
    QModbusReply *reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;

    readReplyHandle(reply);
}

void FanHomeFrame::on_setPIBtn_clicked()
{
    //![1] Init data
    FanPIParameters _PIPara;
    _PIPara.m_speedKp = (quint16)(ui->SpinBox_speedPRW->value() * 10000);
    _PIPara.m_speedKi = (quint16)(ui->SpinBox_speedIRW->value() * 10000);
    _PIPara.m_idKp = (quint16)(ui->SpinBox_idPRW->value() * 10000);
    _PIPara.m_idKi = (quint16)(ui->SpinBox_idIRW->value() * 10000);
    _PIPara.m_iqKp = (quint16)(ui->SpinBox_iqPRW->value() * 10000);
    _PIPara.m_iqKi = (quint16)(ui->SpinBox_iqIRW->value() * 10000);

    int _address = ui->spinBox_InitAdress->value();//!< Server address
    quint16 *buff = (quint16 *)&_PIPara;

    //![2] Pack data to a QModbusDataUnit
    int startAddress = g_mPIParaRegisterAddress;
    int numberOfEntries = g_mPIParaRegisterCount;
    QModbusDataUnit writeUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, startAddress, numberOfEntries);
    for (uint i = 0; i < writeUnit.valueCount(); i++) {

        writeUnit.setValue(i, *buff++);
    }

     MainWindow::getInstance()->sendRtuWriteRequest(writeUnit, _address);
}
