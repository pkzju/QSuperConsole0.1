#ifndef FANHOMEFRAME_H
#define FANHOMEFRAME_H

#include <QFrame>
#include "fanmotor/fpublic.h"
#include <QModbusReply>

#include "qcustomplot/qcustomplot.h"

class QButtonGroup;
class QTableWidget;

namespace Ui {
class FanHomeFrame;
}

class FanHomeFrame : public QFrame
{
    Q_OBJECT

public:
    explicit FanHomeFrame(QWidget *parent = 0);
    ~FanHomeFrame();
    void setGroup(QVector<FanGroupInfo *> *motorGroups, int start, int number);

public slots:

private slots:
    void enterGroup(int id);
    void on_previous1_clicked();
    void on_next1_clicked();
    void on_previous2_clicked();

    void on_spinBox_motorNum_valueChanged(int arg1);
    void on_settingsButton_clicked();
    void on_clearButton_clicked();
    void changeGroup(FanGroupInfo *group);
    void changeMotor(int id);

    //! Init set part
    void on_pushButton_InitSetF_clicked();
    void on_pushButton_InitSetG_clicked();
    void on_pushButton_InitSetA_clicked();
    void on_pushButton_InitRead_clicked();
    void readInitReady();//!< For modbus rtu mode

    //! Read rated data part
    void on_initializeButton_clicked();
    void on_initializeGButton_clicked();
    void on_initializeAButton_clicked();
    void readInitFGAReady();//!< For modbus rtu mode

    //! Change motor state part
    void onRunStateButtonClicked();//!< Change motor state buttons slot

    //! Monitor part
    void on_checkBox_monitorASW_stateChanged(int arg1);
    void monitor_stateChanged(int state);
    void monitorSigleStateChange(int state);
    void monitorTimer_update();
    void monitorReadReady();

    bool readReplyHandle(QModbusReply *reply);
    QMotor *findMotor(quint16 address);
    quint16 *findRegister(QMotor *motor, quint16 registerAddress);


    void on_checkBox_stateChanged(int arg1);
    void on_pushButton_InitRead_2_clicked();
    void on_pushButton_InitSetF_2_clicked();
    void on_readPIBtn_clicked();
    void readPIReady();
    void on_setPIBtn_clicked();

private:
    Ui::FanHomeFrame *ui;
    QButtonGroup *btnGroup = Q_NULLPTR;
    QTableWidget *firstTable = Q_NULLPTR;
    QTableWidget *secondTable = Q_NULLPTR;

    FanGroupInfo *currentGroup = Q_NULLPTR;
    QButtonGroup *fans = Q_NULLPTR;

    QVector<FanGroupInfo*> *allGroups;
    QVector<FanGroupInfo*> groups;
    QMotor *currentMotor = Q_NULLPTR;
    bool m_monitorState = false;
    quint16 m_specialMotorAdd = 0;
    int m_monitorTimerPeriod = 0;
    int m_monitorTimeroutCount = 0;

    QCPDataMap *m_speedRefData;
    QCPDataMap *m_speedFbkData;
    QCPDataMap *m_idRefData;
    QCPDataMap *m_idFbkData;
    QCPDataMap *m_iqRefData;
    QCPDataMap *m_iqFbkData;


    void initUi();
    void updateMotorUi(int arg1);
    void setupRealtimeDataDemo(QCustomPlot *customPlot);
    void realtimeDataSlot();
};

#endif // FANHOMEFRAME_H
