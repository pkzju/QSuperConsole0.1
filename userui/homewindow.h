#ifndef HOMEWINDOW_H
#define HOMEWINDOW_H

#include <QCanBusFrame>
#include <QModbusDataUnit>
#include <QtCore>
#include <QModbusTcpServer>

#include "dialogs/settingdialog.h"
#include"userui/serialportsettingsdialog.h"
#include "fanmotor/fpublic.h"

class QPushButton;
class QStatusBar;

namespace Ui {
class homewindow;
}


class FModbusTcpServer : public QModbusTcpServer{
    Q_OBJECT

public:
    explicit FModbusTcpServer(QObject *parent = nullptr);
    QModbusResponse processRequest(const QModbusPdu &request) Q_DECL_OVERRIDE;

};

class homewindow : public QWidget
{
    Q_OBJECT

public:
    explicit homewindow(QWidget *parent = 0);
    ~homewindow();

    QStatusBar *statusBar();

    QVector<FanGroupInfo *> *groups();

    static homewindow *getInstance();
    static void deleteInstance();
    QModbusResponse processRequest(const QModbusPdu &request);
private:
    QModbusResponse processReadHoldingRegistersRequest(const QModbusRequest &rqst);
    QModbusResponse readBytes(const QModbusPdu &request, QModbusDataUnit::RegisterType unitType);
    QModbusResponse processWriteMultipleRegistersRequest(const QModbusRequest &request);
    QModbusResponse processWriteSingleRegisterRequest(const QModbusRequest &rqst);
    QModbusResponse writeSingle(const QModbusPdu &request, QModbusDataUnit::RegisterType unitType);
private slots:

    void on_spinBox_groupNum_valueChanged(int arg1);

    void button_group_clicked();

    void on_connectButton_clicked();
    void onStateChanged(int state);
    void handleDeviceError(QModbusDevice::Error newError);

private:
    Ui::homewindow *ui;
    QVector<QPushButton*> mGroup;
    QVector<FanGroupInfo*> mGroups;
    FanGroupInfo *mCurrentGroupInfo;
    FModbusTcpServer *modbusDevice;

    CommunicationMode m_communication;
    SerialPortSettings::Settings mSerialPortSettings;


    static homewindow* s_Instance;



};

#endif // HOMEWINDOW_H
