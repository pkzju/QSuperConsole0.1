#ifndef MODBUSUI_H
#define MODBUSUI_H

#include <QWidget>
#include <QModbusDataUnit>

QT_BEGIN_NAMESPACE
class QModbusClient;
class QModbusReply;

namespace Ui {
class ModbusUi;
}
QT_END_NAMESPACE

class QIntValidator;

class WriteRegisterModel;

class ModbusUi : public QWidget
{
    Q_OBJECT

public:
    explicit ModbusUi(QWidget *parent = 0);
    ~ModbusUi();
    static ModbusUi *getInstance();
    static void deleteInstance();

    QModbusClient *getModbusDevice();

private slots:
    void checkCustomBaudRatePolicy(int idx);
    void checkCustomDevicePathPolicy(int idx);
    void on_searchButton_clicked();

    void on_connectType_currentIndexChanged(int);
    void onStateChanged(int state);


    void on_readButton_clicked();
    void readReady();

    void on_writeButton_clicked();
    void on_readWriteButton_clicked();
    void on_writeTable_currentIndexChanged(int);

public slots:
    void on_connectButton_clicked();

private:
    void fillPortsInfo();
    void fillPortsParameters();

    QModbusDataUnit readRequest() const;
    QModbusDataUnit writeRequest() const;


private:
    Ui::ModbusUi *ui;
    QModbusReply* lastRequest;
    QModbusClient* modbusDevice;
    WriteRegisterModel *writeModel;
    QIntValidator *intValidator;
    static ModbusUi* instance;

    quint8 ROW_COUNT;//number of registers
};

#endif // MODBUSUI_H
