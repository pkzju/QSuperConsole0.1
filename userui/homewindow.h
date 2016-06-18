#ifndef HOMEWINDOW_H
#define HOMEWINDOW_H

#include <QCanBusFrame>
#include <QModbusDataUnit>
#include <QtCore>
#include "dialogs/settingdialog.h"

#include "fanmotor/fpublic.h"

class QPushButton;
class QModbusClient;
class QStatusBar;

namespace Ui {
class homewindow;
}

class homewindow : public QWidget
{
    Q_OBJECT

public:
    explicit homewindow(QWidget *parent = 0);
    ~homewindow();

    QStatusBar *statusBar();

    QModbusClient *getModbusDevice();

    QVector<FanGroupInfo *> *groups();

    static homewindow *getInstance();

signals:
    void connectStateChanged(int state);//1:connect 0:disconnect


private slots:
    void onStateChanged(int state);

    void on_spinBox_groupNum_valueChanged(int arg1);

    void button_group_clicked();

    void on_connectButton_clicked();

    void on_disconnectButton_clicked();

    void on_settingsButton_clicked();

private:
    Ui::homewindow *ui;
    QVector<QPushButton*> mGroup;
    QVector<FanGroupInfo*> mGroups;
    FanGroupInfo *mCurrentGroupInfo;
    CommunicationMode m_communication;
    SerialPortSettings::Settings mSerialPortSettings;

    QModbusClient* modbusDevice;



    static homewindow* s_Instance;
};

#endif // HOMEWINDOW_H
