#ifndef FANMOTORUI_H
#define FANMOTORUI_H

#include <QWidget>
#include <QModbusDataUnit>
#include <QCanBusFrame>
#include <QModbusTcpServer>

#include "fanmotor/qmotor.h"
#include "canopen/canfestival.h"
#include "thread/canthread.h"
#include "fanmotor/fpublic.h"


QT_BEGIN_NAMESPACE
class QModbusClient;
class QModbusReply;
class QTimer;
class QStatusBar;
class QTableWidget;

namespace Ui {
class FanMotorUi;
}
QT_END_NAMESPACE

class QcwIndicatorLamp;
class QMotor;
struct FanMotorController;

class HModbusTcpServer : public QModbusTcpServer{
    Q_OBJECT

public:
    explicit HModbusTcpServer(QObject *parent = nullptr);
    QModbusResponse processRequest(const QModbusPdu &request) Q_DECL_OVERRIDE;

};

class FanMotorUi : public QWidget
{
    Q_OBJECT

public:
    explicit FanMotorUi(QWidget *parent = 0);
    ~FanMotorUi();

public:
    static FanMotorUi *getS_Instance();
    static void deleteInstance();

    //! For CANOpen
    void heartbeatError(CO_Data *d, unsigned char heartbeatID);
    void post_sync(CO_Data *d);
    void post_SlaveStateChange(CO_Data *d, unsigned char nodeId, e_nodeState newNodeState);

    QStatusBar *statusBar();//!< Get main windows status bar

    void changeGroup(FanGroupInfo *group);//!< Change group to show

    QModbusResponse processRequest(const QModbusPdu &request);//!< For modbus tcp server to process request

private slots:

    //! CAN frame show and handle
    void messageShow(const QString &s);
    void messageHandle(const QCanBusFrame &frame);

    //! Some operate view response part
    void on_connectButton_clicked();
    void on_disconnectButton_clicked();
    void onStateChanged(int state);
    void on_spinBox_motorNum_valueChanged(int arg1);
    void on_settingsButton_clicked();
    void on_clearButton_clicked();

    //! Init set part
    void on_pushButton_InitSetF_clicked();
    void on_pushButton_InitSetG_clicked();
    void on_pushButton_InitSetA_clicked();
    void on_pushButton_InitRead_clicked();
    void readInitReady();//!< For modbus rtu mode

    //! Monitor part
    void on_groupmonitorButton_clicked();
    void on_checkBox_monitorASW_stateChanged(int arg1);
    void monitor_stateChanged(int state);
    void monitorSigleStateChange(int state);
    void monitorTimer_update();
    void monitorReadReady();

    //! Read rated data part
    void on_initializeButton_clicked();
    void on_initializeGButton_clicked();
    void on_initializeAButton_clicked();
    void readInitFGAReady();//!< For modbus rtu mode

    //! Change motor state part
    void onRunStateButtonClicked();//!< Change motor state buttons slot

    //! For sigle motor ui part
    void onMotorStartButtonClicked();//!< Start motor
    void onMotorStopButtonClicked(); //!< Stop motor
    void onFanButtonclicked();       //!< Show sigle motor ui
    void onSigleMotorInitSetClicked(QTableWidget *table, quint16 data);//!< Init set
    void onSetPI(FanPIParameters &fpi);
    void onReadPI();
    void readPIReady();//!< For modbus rtu mode

    //! Tcp server state change slot
    void onTcpServerStateChanged(int state);

    //! Let local modbus rtu master send read / write request to slave
    void readFromMotor(quint16 motorAdd, quint16 registerAdd, quint16 count);
    void writeToMotor(quint16 motorAdd, QModbusDataUnit unit);

signals:
    void updatePlotUi(FanMotorController motorctr);

    //! For sigle motor ui
    void updateSigleMotor(int arg1);
    void updateSigleMotorState(bool state);

    //! Emit this signal let local modbus tcp client send write request to remote modbus tcp server
    void writeMotorRegister(quint16 motorAdd, quint16 registerAdd, quint16 count);

private:
    //! Tcp server process request part
    QModbusResponse processReadHoldingRegistersRequest(const QModbusRequest &rqst);
    QModbusResponse readBytes(const QModbusPdu &request, QModbusDataUnit::RegisterType unitType);
    QModbusResponse processWriteMultipleRegistersRequest(const QModbusRequest &request);
    QModbusResponse processWriteSingleRegisterRequest(const QModbusRequest &rqst);

    //! Frequently used fuction part
    void sendCommand(const FCommandRegister &fcr, quint16 serverAddress);
    void sendMobusReadRequest(quint16 motorAdd, quint16 registerAdd, quint16 count);
    QMotor *findMotor(quint16 address);
    quint16 *findRegister(QMotor *motor, quint16 registerAddress);
    bool sendModbusWriteRequest(QModbusClient *modbusDevice, QModbusDataUnit unit, quint16 serverAddress);
    bool readReplyHandle(QModbusReply *reply);

private:
    static FanMotorUi *s_Instance;

    Ui::FanMotorUi *ui;
    QVector<FanGroupInfo *> *m_groups;
    QModbusClient *modbusRtuDevice;//!< Modbus RTU device
    QVector<QMotor *> *m_motors;
    QTimer *m_pollingTimer;
    s_BOARD *m_masterBoard;
    CanThread *m_canThread;
    FanGroupInfo *m_currentGroup;
    QModbusClient *modbusTcpDevice;//!< Modbus tcp client device
    HModbusTcpServer *modbusTcpServer;//!< Modbus tcp server device



    int m_realTimeServerAddress;
    int m_currentServerAddress;
    int m_startServerAddress;
    int m_motorNum;
    CommunicationMode m_communication;
    bool m_CANopenStart = false;
    bool m_monitorState = false;
    int m_monitorTimerPeriod = 0;
    int m_monitorTimeroutCount = 0;
    QVector<QPushButton*> m_motorButtons;
    quint16 m_specialMotorAdd = 0;

};

#endif // FANMOTORUI_H
