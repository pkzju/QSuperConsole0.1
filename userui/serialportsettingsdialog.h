/****************************************************************************
**
**
****************************************************************************/

#ifndef SERIALPORTSETTINGSDIALOG_H
#define SERIALPORTSETTINGSDIALOG_H

#include <QDialog>
#include <QtSerialPort/QSerialPort>


QT_USE_NAMESPACE

QT_BEGIN_NAMESPACE

namespace Ui {
class SerialPortSettingsDialog;
}

class QIntValidator;

QT_END_NAMESPACE

class SerialPortSettings
{
public:
    SerialPortSettings() {}
public:
    struct Settings {
        QString name;
        qint32 baudRate;
        QString stringBaudRate;
        QSerialPort::DataBits dataBits;
        QString stringDataBits;
        QSerialPort::Parity parity;
        QString stringParity;
        QSerialPort::StopBits stopBits;
        QString stringStopBits;
        QSerialPort::FlowControl flowControl;
        QString stringFlowControl;
    };
};


class SerialPortSettingsDialog : public QWidget
{
    Q_OBJECT

public:

    explicit SerialPortSettingsDialog(QWidget *parent = 0);
    ~SerialPortSettingsDialog();

    static SerialPortSettingsDialog* getInstance();
    SerialPortSettings::Settings settings() const;
    void setSettings(SerialPortSettings::Settings s);

private slots:

    void apply();
    void checkCustomBaudRatePolicy(int idx);
    void checkCustomDevicePathPolicy(int idx);

    void on_searchButton_clicked();

    void on_Button_apply_clicked();

signals:
    void emitApply();

private:
    void fillPortsParameters();
    void fillPortsInfo();
    void updateSettings();

private:
    Ui::SerialPortSettingsDialog *ui;

    static SerialPortSettingsDialog *instance;
    QIntValidator *intValidator;

    SerialPortSettings::Settings currentSettings;

};

#endif // SETTINGSDIALOG_H
