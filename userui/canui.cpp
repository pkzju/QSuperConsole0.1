#include "canui.h"
#include "ui_canui.h"

#include <QTime>


#include <QDebug>

extern CO_Data master_Data;

CANUi *CANUi::s_Instance = nullptr;





void NMTThread::run()
{
    QString result;

    master_Data.NMTable[this->node_id] = Unknown_state;

    QTime time;
    time.start();

    e_nodeState state = Unknown_state;
    while(time.elapsed() < this->timeout_ms)
    {
        state = getNodeState(&master_Data, this->node_id);
        if (state != Unknown_state)
            break;

    }

    switch (state) {
    case Initialisation:
        result = "Initialisation";
        break;
    case Disconnected:
        result = "Disconnected";
        break;
    case Connecting:
        result = "Connecting";
        break;
    case Pre_operational:
        result = "Pre_operational";
        break;
    case Operational:
        result = "Operational";
        break;
    case Stopped:
        result = "Stopped";
        break;
    default:
        result = "Unknown_state";
        break;
    }
qDebug()<<result;
    emit resultReady(result);
}


void NMTThread::doPollingState(UNS8 id, int ms) {
    if(!this->isRunning()){
        this->node_id = id;
        this->timeout_ms = ms;
        this->start();
    }else{
        this->quit();
        this->node_id = id;
        this->timeout_ms = ms;
        this->start();
    }
}


void InitNodes(CO_Data* d, UNS32 id)
{
    setNodeId(&master_Data,0x7f);
    setState(&master_Data, Initialisation);
    setState(&master_Data, Operational);
}
void Exit(CO_Data* d, UNS32 id)
{
//    masterSendNMTstateChange (&master_Data, 0x01, NMT_Stop_Node);
    setState(&master_Data, Stopped);
}
void CANUi::CANopenStart(){

    if(!m_CANopenState){
        m_CANopenState = true;
        TimerInit();
        StartTimerLoop(&InitNodes);
    }

}
void CANUi::CANopenStop(){
    if(m_CANopenState){
        m_CANopenState = false;
        StopTimerLoop(&Exit);
    }
}

void CANUi::resetCAN()
{
    if(ui->pushButton_Close->isEnabled())
        on_pushButton_Close_clicked();
}

CANUi::CANUi(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::CANUi)
  ,m_masterBoard(new s_BOARD{})
  ,m_canThread(CanThread::getInstance())
{
    ui->setupUi(this);
    initData();
    initUI();
    initThread();
    initConnect();
}

void CANUi::initData()
{
    m_masterBoard->busname = const_cast<char *>("master");
    m_masterBoard->baudrate = const_cast<char *>("125000");
    m_CANopenState = false;
    m_CANState = false;

    {
        UNS32 _destData = 0;
        UNS32 _dataSize = sizeof(UNS32);
        UNS8 _dataType = uint32;
        if(OD_SUCCESSFUL != readLocalDict(&master_Data, (UNS16)0x1005, (UNS8)0, (UNS32*)&_destData, &_dataSize, &_dataType, 1 )){
            ui->textBrowser->append("readLocalDict 0x1005 fail !");
        }
        else
            ui->textBrowser->append(tr("readLocalDict 0x1005 :0x%1 !").arg(QString::number((quint32)_destData,16)));
    }


}

void CANUi::initUI()
{
    ui->pushButton_Send->setEnabled(false);

    ui->pushButton_Close->setEnabled(false);
    ui->Checkb_CANopen->setEnabled(false);

    ui->pushButton_getNodeState->setEnabled(false);
    ui->pushButton_setNodeState->setEnabled(false);
    ui->pushButton_guardSet->setEnabled(false);
    ui->pushButton_pdoSend->setEnabled(false);
    ui->pushButton_sdoRead->setEnabled(false);
    ui->pushButton_sdoSet->setEnabled(false);



    ui->comboBox_nodeState->addItem("Start_Node", NMT_Start_Node);
    ui->comboBox_nodeState->addItem("Stop_Node", NMT_Stop_Node);
    ui->comboBox_nodeState->addItem("Enter_PreOperational", NMT_Enter_PreOperational);
    ui->comboBox_nodeState->addItem("Reset_Node", NMT_Reset_Node);
    ui->comboBox_nodeState->addItem("Reset_Comunication", NMT_Reset_Comunication);

    ui->comboBox_sdoDatatype->addItem("uint8", uint8);
    ui->comboBox_sdoDatatype->addItem("uint16", uint16);
    ui->comboBox_sdoDatatype->addItem("uint32", uint32);
    ui->comboBox_datatype->addItem("uint8", uint8);
    ui->comboBox_datatype->addItem("uint16", uint16);
    ui->comboBox_datatype->addItem("uint32", uint32);

}

void CANUi::initThread()
{

}

void CANUi::initConnect()
{

}

CANUi::~CANUi()
{
    if(ui->pushButton_Close->isEnabled())
        on_pushButton_Close_clicked();

    delete ui;
}

CANUi *CANUi::getS_Instance()
{
    if(!s_Instance)
    {
        s_Instance = new CANUi;
    }
    return s_Instance;
}


void CANUi::on_pushButton_Open_clicked()
{
    if(m_CANState){
        ui->textBrowser->append("already opened !");
        return;
    }

    if(!canOpen(m_masterBoard, &master_Data)){
        ui->textBrowser->append("open failed !");
        return;
    }

    m_CANState = true;
    ui->textBrowser->append("open success !");

    connect(m_canThread, SIGNAL(message(QString)),
            this, SLOT(messageShow(QString)));

    connect(m_canThread, SIGNAL(message(QCanBusFrame)),
            this, SLOT(messageShow(QCanBusFrame)));

    m_canThread->mStart(false);//open canport and close canopen

    ui->textBrowser->append(tr("ObjdictSize: %1").arg(*master_Data.ObjdictSize));


    ui->pushButton_Open->setEnabled(false);
    ui->pushButton_Close->setEnabled(true);

    ui->Checkb_CANopen->setEnabled(true);//canopen open
    ui->pushButton_Send->setEnabled(true);


}

void CANUi::on_pushButton_Close_clicked()
{
    if(!m_CANState){
        return;
    }

    if(ui->Checkb_CANopen->isChecked()){//first
        ui->Checkb_CANopen->click();    //close canopen
    }

    m_canThread->mStop();               //then close thread
    disconnect(m_canThread, SIGNAL(message(QString)),
            this, SLOT(messageShow(QString)));

    disconnect(m_canThread, SIGNAL(message(QCanBusFrame)),
            this, SLOT(messageShow(QCanBusFrame)));

    canClose(&master_Data);            //final close canport


    m_CANState = false;
    ui->textBrowser->append("close success !");

    ui->pushButton_Close->setEnabled(false);
    ui->pushButton_Open->setEnabled(true);



    ui->Checkb_CANopen->setEnabled(false);
    ui->pushButton_Send->setEnabled(false);


}

void CANUi::on_pushButton_Clear_clicked()
{
    ui->textBrowser->clear();
}

void CANUi::messageShow(const QString &s)
{

    ui->textBrowser->append(s);
}

void CANUi::messageShow(const QCanBusFrame &frame)
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

    if (frame.frameType() == QCanBusFrame::RemoteRequestFrame) {
        ui->requestList->addItem(view);
    } else if (frame.frameType() == QCanBusFrame::ErrorFrame) {
        ui->errorList->addItem(view);
    } else {
        ui->listWidget->addItem(view);
    }

}



void CANUi::on_pushButton_Send_clicked()
{
    QByteArray writings = ui->lineEdit->displayText().toUtf8();//data

    QCanBusFrame frame;
    const int maxPayload = ui->fdBox->checkState() ? 64 : 8;//datasize
    int size = writings.size();
    if (size > maxPayload)
        size = maxPayload;
    writings = writings.left(size);
    frame.setPayload(writings);

    qint32 id = ui->idEdit->displayText().toInt();//id
    if (!ui->EFF->checkState() && id > 2047) //11 bits
        id = 2047;

    frame.setFrameId(id);
    frame.setExtendedFrameFormat(ui->EFF->checkState());

    if (ui->remoteFrame->isChecked())
        frame.setFrameType(QCanBusFrame::RemoteRequestFrame);
    else if (ui->errorFrame->isChecked())
        frame.setFrameType(QCanBusFrame::ErrorFrame);
    else
        frame.setFrameType(QCanBusFrame::DataFrame);


    Message m{0};
    m.cob_id = frame.frameType();
    if(frame.frameType() == QCanBusFrame::RemoteRequestFrame)
        m.rtr = 1;
    m.len = frame.payload().size();
    for(quint8 i = 0 ; i < m.len ; i++){
        m.data[i] = frame.payload().at(i);
    }
    usbCanSend(0, &m);


}


void CANUi::on_pushButton_setNodeState_clicked()
{
    UNS8 id = ui->spinBox_NMTnodeID->value();
    UNS8 state = ui->comboBox_nodeState->currentData().value<UNS8>();
    masterSendNMTstateChange(&master_Data, id, state);
}

void CANUi::on_pushButton_getNodeState_clicked()
{
    UNS8 id = ui->spinBox_NMTnodeID->value();
    masterRequestNodeState(&master_Data, id);
    NMTThread *workerThread = new NMTThread;//new polling thread
    connect(workerThread, &NMTThread::resultReady, ui->nowNodeState, &QLineEdit::setText);
    connect(workerThread, &NMTThread::finished, workerThread, &QObject::deleteLater);
    workerThread->doPollingState(id, 1000);//start thread
}

void CANUi::on_Checkb_CANopen_stateChanged(int arg1)
{
    if(arg1){//if checked
        m_canThread->mStart(true);//open canopen
        CANopenStart();//init timer and status

        ui->pushButton_getNodeState->setEnabled(true);
        ui->pushButton_setNodeState->setEnabled(true);
        ui->pushButton_guardSet->setEnabled(true);
        ui->pushButton_pdoSend->setEnabled(true);
        ui->pushButton_sdoRead->setEnabled(true);
        ui->pushButton_sdoSet->setEnabled(true);
    }
    else{
        m_canThread->mStart(false);//close canopen but not canport
        ui->pushButton_getNodeState->setEnabled(false);
        ui->pushButton_setNodeState->setEnabled(false);
        ui->pushButton_guardSet->setEnabled(false);
        ui->pushButton_pdoSend->setEnabled(false);
        ui->pushButton_sdoRead->setEnabled(false);
        ui->pushButton_sdoSet->setEnabled(false);

        CANopenStop();//close timer and set stop status
    }

}

void CANUi::on_pushButton_guardSet_clicked()
{
    if(ui->radiob_heartBeat->isChecked()){
        int _nodeNum = ui->spinBox_nodeNum->value();
        UNS32 _time = ui->spinBox_guardTime->value();
        for(int i = 1; i <= _nodeNum; i++){
            {//0x1016 :   Consumer Heartbeat Time  nodeid(bit 23 : 16) + time(bit 15 : 0) ms
                UNS32 _sourceData = _time + (i << 16);
                UNS32 _dataSize = sizeof(UNS32);
                if(OD_SUCCESSFUL != writeLocalDict(&master_Data, (UNS16)0x1016, (UNS8)i, (UNS32*)&_sourceData, &_dataSize, 1 )){
                    ui->textBrowser->append(tr("writeLocalDict 0x1016 %1: fail !").arg(i));
                }
            }
        }
    }
    else {
        int _nodeNum = ui->spinBox_nodeNum->value();
        UNS32 _time = 0;
        for(int i = 1; i <= _nodeNum; i++){
            {//0x1016 :   Consumer Heartbeat Time  nodeid(bit 23 : 16) + time(bit 15 : 0) ms
                UNS32 _sourceData = _time + (i << 16);
                UNS32 _dataSize = sizeof(UNS32);
                if(OD_SUCCESSFUL != writeLocalDict(&master_Data, (UNS16)0x1016, (UNS8)i, (UNS32*)&_sourceData, &_dataSize, RW )){
                    ui->textBrowser->append(tr("writeLocalDict 0x1016 %1: fail !").arg(i));
                }
            }
        }
    }
}

void CANUi::on_checkBox_sync_toggled(bool checked)
{
    if(checked){
        stopSYNC(&master_Data);
        {//0x1005 :   SYNC COB ID gen(bit 30) COB-ID : 80
            UNS32 _sourceData = 0x40000080;
            UNS32 _dataSize = sizeof(UNS32);
            if(OD_SUCCESSFUL != writeLocalDict(&master_Data, (UNS16)0x1005, (UNS8)0, &_sourceData, &_dataSize, 1 )){
                ui->textBrowser->append("writeLocalDict 0x1005 fail !");
            }
        }

        {//0x1006 :   SYNC Cycle Period   us
            UNS32 _sourceData = ui->spinBox_syncTime->value();
            ui->textBrowser->append(tr("%1").arg(_sourceData));
            UNS32 _dataSize = sizeof(UNS32);
            if(OD_SUCCESSFUL != writeLocalDict(&master_Data, (UNS16)0x1006, (UNS8)0, &_sourceData, &_dataSize, 1 )){
                ui->textBrowser->append("writeLocalDict 0x1006 fail !");
            }
        }
        startSYNC(&master_Data);
    }
    else{
        stopSYNC(&master_Data);
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
    }
}

void CANUi::on_checkBox_pdoReceive_toggled(bool checked)
{
    if(checked){
        {//0x1600 :   RPDO1 COB-ID 0x180 + nodeid
            UNS32 _sourceData = ui->spinBox_rpdoCOB->value() + ui->spinBox_pdonodeID->value();
            UNS32 _dataSize = sizeof(UNS32);
            if(OD_SUCCESSFUL != writeLocalDict(&master_Data, (UNS16)0x1400, (UNS8)1, (UNS32*)&_sourceData, &_dataSize, 1 )){
                ui->textBrowser->append("writeLocalDict 0x1400 fail !");
            }
        }
    }else{
        {//0x1600 :   RPDO1 COB-ID 0x180 + nodeid
            UNS32 _sourceData = 0;
            UNS32 _dataSize = sizeof(UNS32);
            if(OD_SUCCESSFUL != writeLocalDict(&master_Data, (UNS16)0x1400, (UNS8)1, (UNS32*)&_sourceData, &_dataSize, 1 )){
                ui->textBrowser->append("writeLocalDict 0x1400 fail !");
            }
        }

    }
}

void CANUi::on_pushButton_dicSet_clicked()
{
    UNS32 _dataSize;
    UNS8 _dataType = ui->comboBox_datatype->currentData().value<UNS8>();

    if(_dataType == uint8){
        _dataSize = sizeof(UNS8);
    }
    else if(_dataType == uint16){
        _dataSize = sizeof(UNS16);
    }
    else{
        _dataSize = sizeof(UNS32);
    }

    UNS32 _sourceData = ui->spinBox_dicData->value();
    UNS16 _index = ui->spinBox_index->value();
    UNS8 _subIndex = ui->spinBox_subInxdex->value();

    if(OD_SUCCESSFUL != writeLocalDict(&master_Data, _index, _subIndex, &_sourceData, &_dataSize, 1 )){
        ui->textBrowser->append("writeLocalDict fail !");
    }
}

void CANUi::on_pushButton_dicRead_clicked()
{
    UNS32 _dataSize;
    UNS8 _dataType = ui->comboBox_datatype->currentData().value<UNS8>();

    if(_dataType == uint8){
        _dataSize = sizeof(UNS8);
    }
    else if(_dataType == uint16){
        _dataSize = sizeof(UNS16);
    }
    else{
        _dataSize = sizeof(UNS32);
    }

    UNS32 _destData;

    UNS16 _index = ui->spinBox_index->value();
    UNS8 _subIndex = ui->spinBox_subInxdex->value();

    if(OD_SUCCESSFUL != readLocalDict(&master_Data, _index, _subIndex, &_destData, &_dataSize, &_dataType, 1 )){
        ui->textBrowser->append("readLocalDict  fail !");
    }
    else
        ui->spinBox_dicData->setValue(_destData);


}


void WriteResultNetworkDict(CO_Data* d, UNS8 nodeId)
{
    UNS32 abortCode;

    if(getWriteResultNetworkDict (d, nodeId, &abortCode) != SDO_FINISHED){

    }

    /* Finalise last SDO transfer with this node */
    closeSDOtransfer(&master_Data, nodeId, SDO_CLIENT);

}


void ReadResultNetworkDict(CO_Data* d, UNS8 nodeId)
{
    CANUi *ui = CANUi::getS_Instance();
    ui->mReadResultNetworkDict(d, nodeId);
}


void CANUi::mReadResultNetworkDict(CO_Data* d, UNS8 nodeId)
{
    UNS32 abortCode;

    UNS32 _dataSize;
    UNS8 _dataType = this->ui->comboBox_sdoDatatype->currentData().value<UNS8>();

    if(_dataType == uint8){
        _dataSize = sizeof(UNS8);
    }
    else if(_dataType == uint16){
        _dataSize = sizeof(UNS16);
    }
    else{
        _dataSize = sizeof(UNS32);
    }
    UNS32 data;

    if(getReadResultNetworkDict (d, nodeId, &data, &_dataSize,
                                      &abortCode)!=SDO_FINISHED){

    }

    if(_dataType == uint8){
        ui->spinBox_sdoData->setValue((UNS8)data);
    }
    else if(_dataType == uint16){
        ui->spinBox_sdoData->setValue((UNS16)data);
    }
    else{
        ui->spinBox_sdoData->setValue((UNS32)data);
    }


    /* Finalise last SDO transfer with this node */
    closeSDOtransfer(&master_Data, nodeId, SDO_CLIENT);

}

void CANUi::on_pushButton_sdoSet_clicked()
{
    UNS8 nodeId = ui->spinBox_sdoID->value();

    {//0x1280 :   SDO1  COB_ID_Client_to_Server  bit30:dyn 0x600 + nodeid
        UNS32 _sourceData =  0x600 + nodeId;
        UNS32 _dataSize = sizeof(UNS32);
        if(OD_SUCCESSFUL != writeLocalDict(&master_Data, (UNS16)0x1280, (UNS8)1, &_sourceData, &_dataSize, 1 )){
            ui->textBrowser->append("writeLocalDict 0x1280 fail !");
        }
    }

    {//0x1280 :   SDO1  COB_ID_Server_to_Client  0x580 + nodeid
        UNS32 _sourceData =  0x580 + nodeId;
        UNS32 _dataSize = sizeof(UNS32);
        if(OD_SUCCESSFUL != writeLocalDict(&master_Data, (UNS16)0x1280, (UNS8)2, &_sourceData, &_dataSize, 1 )){
            ui->textBrowser->append("writeLocalDict 0x1280 fail !");
        }
    }

    {//0x1280 :   Node_ID_of_the_SDO_Server   nodeid
        UNS8 _sourceData = nodeId;
        UNS32 _dataSize = sizeof(UNS8);
        if(OD_SUCCESSFUL != writeLocalDict(&master_Data, (UNS16)0x1280, (UNS8)3, &_sourceData, &_dataSize, 1 )){
            ui->textBrowser->append("writeLocalDict 0x1280 fail !");
        }
    }

    UNS32 _dataSize;
    UNS8 _dataType = ui->comboBox_sdoDatatype->currentData().value<UNS8>();

    if(_dataType == uint8){
        _dataSize = sizeof(UNS8);
    }
    else if(_dataType == uint16){
        _dataSize = sizeof(UNS16);
    }
    else{
        _dataSize = sizeof(UNS32);
    }


    UNS32 _sourceData = ui->spinBox_sdoData->value();
    UNS16 _index = ui->spinBox_sdoIndex->value();
    UNS8 _subIndex = ui->spinBox_sdoSubIndex->value();
    int res = writeNetworkDictCallBack (&master_Data, /*CO_Data* d*/
                                        /**TestSlave_Data.bDeviceNodeId, UNS8 nodeId*/
                                        nodeId, /*UNS8 nodeId*/
                                        _index, /*UNS16 index*/
                                        _subIndex, /*UNS8 subindex*/
                                        _dataSize, /*UNS8 count*/
                                        _dataType, /*UNS8 dataType*/
                                        &_sourceData,/*void *data*/
                                        WriteResultNetworkDict, /*SDOCallback_t Callback*/
                                        0); /* use block mode */
}

void CANUi::on_pushButton_ClearAll_clicked()
{
    ui->listWidget->clear();
}

void CANUi::on_pushButton_sdoRead_clicked()
{
    UNS8 nodeId = ui->spinBox_sdoID->value();

    {//0x1280 :   SDO1  COB_ID_Client_to_Server  bit30:dyn 0x600 + nodeid
        UNS32 _sourceData =  0x600 + nodeId;
        UNS32 _dataSize = sizeof(UNS32);
        if(OD_SUCCESSFUL != writeLocalDict(&master_Data, (UNS16)0x1280, (UNS8)1, &_sourceData, &_dataSize, 1 )){
            ui->textBrowser->append("writeLocalDict 0x1280 fail !");
        }
    }

    {//0x1280 :   SDO1  COB_ID_Server_to_Client  0x580 + nodeid
        UNS32 _sourceData =  0x580 + nodeId;
        UNS32 _dataSize = sizeof(UNS32);
        if(OD_SUCCESSFUL != writeLocalDict(&master_Data, (UNS16)0x1280, (UNS8)2, &_sourceData, &_dataSize, 1 )){
            ui->textBrowser->append("writeLocalDict 0x1280 fail !");
        }
    }

    {//0x1280 :   Node_ID_of_the_SDO_Server   nodeid
        UNS8 _sourceData = nodeId;
        UNS32 _dataSize = sizeof(UNS8);
        if(OD_SUCCESSFUL != writeLocalDict(&master_Data, (UNS16)0x1280, (UNS8)3, &_sourceData, &_dataSize, 1 )){
            ui->textBrowser->append("writeLocalDict 0x1280 fail !");
        }
    }


    UNS16 _index = ui->spinBox_sdoIndex->value();
    UNS8 _subIndex = ui->spinBox_sdoSubIndex->value();
    UNS32 _dataSize;
    UNS8 _dataType = ui->comboBox_sdoDatatype->currentData().value<UNS8>();

    if(_dataType == uint8){
        _dataSize = sizeof(UNS8);
    }
    else if(_dataType == uint16){
        _dataSize = sizeof(UNS16);
    }
    else{
        _dataSize = sizeof(UNS32);
    }

    int res = readNetworkDictCallback (&master_Data,
                                       nodeId,
                                    _index,
                                    _subIndex,
                                    _dataType,
                                    ReadResultNetworkDict,
                                    0);


}
