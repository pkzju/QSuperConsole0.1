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

class SerialPortSettingsDialog : public QWidget
{
    Q_OBJECT

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

    explicit SerialPortSettingsDialog(QWidget *parent = 0);
    ~SerialPortSettingsDialog();

    static SerialPortSettingsDialog* getInstance();
    Settings settings() const;
    void setSettings(SerialPortSettingsDialog::Settings s);

private slots:
    void showPortInfo(int idx);
    void apply();
    void checkCustomBaudRatePolicy(int idx);
    void checkCustomDevicePathPolicy(int idx);

    void on_searchButton_clicked();

    void on_Button_Close_clicked();

    void on_Button_Open_clicked();

signals:
    void emitApply(int idx);

private:
    void fillPortsParameters();
    void fillPortsInfo();
    void updateSettings();

private:
    Ui::SerialPortSettingsDialog *ui;
    Settings currentSettings;
    static SerialPortSettingsDialog *instance;
    QIntValidator *intValidator;
//    SerialPortThread *mySerialPortThread;
};

#endif // SETTINGSDIALOG_H
