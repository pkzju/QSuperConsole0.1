#ifndef FANMOTORUI_H
#define FANMOTORUI_H

#include <QWidget>
#include <QModbusDataUnit>
#include <QCanBusFrame>

#include "fanmotor/fanmotor.h"
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



class FanMotorUi : public QWidget
{
    Q_OBJECT

public:
    explicit FanMotorUi(QWidget *parent = 0);
    ~FanMotorUi();

public:

    enum PollingState:char{
        Stop,
        SingleMotor,
        MultiMotor,
        Searching,
    };
    static FanMotorUi *getS_Instance();
    static void deleteInstance();
    void heartbeatError(CO_Data *d, unsigned char heartbeatID);
    void post_sync(CO_Data *d);
    void post_SlaveStateChange(CO_Data *d, unsigned char nodeId, e_nodeState newNodeState);

    QStatusBar *statusBar();
    void changeGroup(FanGroupInfo *group);



private slots:

    void messageShow(const QString &s);
    void messageHandle(const QCanBusFrame &frame);

    void on_connectButton_clicked();
    void on_disconnectButton_clicked();
    void onStateChanged(int state);
    void on_spinBox_motorNum_valueChanged(int arg1);
    void on_settingsButton_clicked();
    void on_pushButton_InitSetF_clicked();
    void on_pushButton_InitSetG_clicked();
    void on_pushButton_InitSetA_clicked();
    void on_pushButton_InitRead_clicked();
    void readInitReady();
    void on_groupmonitorButton_clicked();
    void on_checkBox_monitorGSW_stateChanged(int arg1);
    void on_checkBox_monitorASW_stateChanged(int arg1);
    void monitor_stateChanged(int state);
    void monitorSigleStateChange(int state);
    void monitorTimer_update();
    void monitorReadReady();

    void on_clearButton_clicked();

    void on_initializeButton_clicked();
    void on_initializeGButton_clicked();
    void on_initializeAButton_clicked();
    void readInitFGAReady();

    void onRunStateButtonClicked();
    void onMotorStartButtonClicked();//for sigle motor ui
    void onMotorStopButtonClicked();//for sigle motor ui
    void onFanButtonclicked();
    void onSigleMotorInitSetClicked(QTableWidget *table, quint16 data);
    void onSetPI();
    void onReadPI();
    void readPIReady();
private:


signals:
    void updatePlotUi(FanMotorController motorctr);
    void updateSigleMotor(int arg1);
    void updateSigleMotorState(bool state);

private:
    static FanMotorUi *s_Instance;

    Ui::FanMotorUi *ui;
    QVector<FanGroupInfo *> *m_groups;
    QModbusClient *modbusDevice;
    QVector<QMotor *> *m_motors;
    QTimer *m_pollingTimer;
    s_BOARD *m_masterBoard;
    CanThread *m_canThread;
    FanGroupInfo *m_currentGroup;
    QTimer *m_monitorTimer;


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
    bool isSigleMotorMonitor = false;





};

#endif // FANMOTORUI_H
