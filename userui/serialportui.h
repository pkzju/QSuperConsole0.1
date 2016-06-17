#ifndef SERIALPORTUI_H
#define SERIALPORTUI_H

#include <QDialog>
#include <QtSerialPort/QSerialPort>
#include "thread/serialportthread.h"

class QIntValidator;

namespace Ui {
class SerialPortUi;
}

class SerialPortUi : public QDialog
{
    Q_OBJECT

private:
    void initData();
    void initUI();
    void initThread();
    void initConnect();

    void fillPortsParameters();
    void fillPortsInfo();
    void updateSettings();

public:
    explicit SerialPortUi(QWidget *parent = 0);
    ~SerialPortUi();
    SerialPortSettings::Settings settings() const;
    void setSettings(SerialPortSettings::Settings s);


private slots:

    void messageShow(const QString &s);

    void showPortInfo(int idx);
    void checkCustomBaudRatePolicy(int idx);
    void checkCustomDevicePathPolicy(int idx);

    void on_searchButton_clicked();

    void on_Button_Close_clicked();

    void on_Button_Open_clicked();

private:
    Ui::SerialPortUi *ui;
    SerialPortThread *mySerialPortThread;
    SerialPortSettings::Settings currentSettings;
    QIntValidator *intValidator;
};

#endif // SERIALPORTUI_H
