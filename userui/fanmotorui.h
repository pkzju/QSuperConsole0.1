#ifndef FANMOTORUI_H
#define FANMOTORUI_H

#include <QWidget>
#include "modbusui.h"
#include <QModbusDataUnit>
#include "fanmotor/fanmotor.h"
#include "canui.h"

QT_BEGIN_NAMESPACE
class QModbusClient;
class QModbusReply;
class QTimer;

namespace Ui {
class FanMotorUi;
}
QT_END_NAMESPACE

class QcwIndicatorLamp;
class QMotor;
struct FanMotorController;

class FanMotorUi : public QWidget
{
    Q_OBJECT

public:
    explicit FanMotorUi(QWidget *parent = 0);
    ~FanMotorUi();

public:
    enum Communication:char{
        Init,
        Tcp,
        Modbus,
        CANbus
    };

    enum PollingState:char{
        Stop,
        SingleMotor,
        MultiMotor,
        Searching,
    };
    static FanMotorUi *getS_Instance();
    void heartbeatError(CO_Data *d, unsigned char heartbeatID);
    void post_sync(CO_Data *d);
    void post_SlaveStateChange(CO_Data *d, unsigned char nodeId, e_nodeState newNodeState);

private slots:

    void messageShow(const QString &s);

    void messageHandle(const QCanBusFrame &frame);

    void pollingTimerUpdate();

    void onModebusStateChanged(int state);

    void on_pushButton_read_clicked();

    void readReady();

    void on_pushButton_write_clicked();

    void on_startButton_clicked();

    void on_searchButton_clicked();

    void on_spinBox_motorNum_valueChanged(int arg1);

    void on_spinBox_currentaddress_valueChanged(int arg1);

    void on_connectButton_clicked();

    void on_pushButton_startMotor_clicked();

    void on_pushButton_stopMotor_clicked();

    void on_disconnectButton_clicked();

    void on_radioButton_modbus_clicked();

    void on_radioButton_can_clicked();

    void on_radioButton_tcp_clicked();


private:
    QModbusDataUnit readRequest() const;
    QModbusDataUnit writeRequest() const;
    void sendOnePolling(int address);

signals:
    void updatePlotUi(FanMotorController motorctr);

private:
    Ui::FanMotorUi *ui;
    ModbusUi *m_modbusUi;
    CANUi *m_canUi;

    QModbusClient* modbusDevice;
    QVector<QMotor*> m_motors;
    QTimer *m_pollingTimer;
    int m_realTimeServerAddress;
    int m_currentServerAddress;
    int m_startServerAddress;
    int m_motorNum;
    PollingState m_pollingState;
    Communication m_communication;

    bool m_CANopenStart;

    s_BOARD *m_masterBoard;
    CanThread *m_canThread;

    static FanMotorUi *s_Instance;

};

#endif // FANMOTORUI_H
