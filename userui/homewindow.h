#ifndef HOMEWINDOW_H
#define HOMEWINDOW_H

#include <QCanBusFrame>
#include <QModbusDataUnit>
#include <QtCore>
#include <QModbusTcpServer>
#include <QModbusTcpClient>

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

    QMotor *findMotor(quint16 address);
private slots:

    void on_spinBox_groupNum_valueChanged(int arg1);

    void button_group_clicked();

    void on_connectButton_clicked();
    void onStateChanged(int state);
    void handleDeviceError(QModbusDevice::Error newError);

    void clientToConnect();
public slots:
    //! Local tcp client send data to remote tcp server
    void writeToMotor(quint16 motorAdd, quint16 registerAdd, quint16 count);

signals:
    //! Let modbus master send read request to slave
    void readMotorRegister(quint16 motorAdd, quint16 registerAdd, quint16 count);
    //! Let modbus master send write request to slave
    void writeMotorRegister(quint16 motorAdd, QModbusDataUnit unit);


private:
    Ui::homewindow *ui;
    QVector<QPushButton*> m_groupBtn;
    QVector<FanGroupInfo*> m_groups;
    FanGroupInfo *mCurrentGroupInfo;
    FModbusTcpServer *modbusDevice;
    QModbusClient* modbusTcpClient;

    CommunicationMode m_communication;
    SerialPortSettings::Settings mSerialPortSettings;


    static homewindow* s_Instance;



};

#endif // HOMEWINDOW_H
