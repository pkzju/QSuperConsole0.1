#ifndef HOMEWINDOW_H
#define HOMEWINDOW_H

#include <QCanBusFrame>
#include <QModbusDataUnit>
#include <QtCore>

#include "dialogs/settingdialog.h"
#include"userui/serialportsettingsdialog.h"
#include "fanmotor/fpublic.h"

class QPushButton;
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

    QVector<FanGroupInfo *> *groups();

    static homewindow *getInstance();
    static void deleteInstance();


private slots:

    void on_spinBox_groupNum_valueChanged(int arg1);

    void button_group_clicked();


private:
    Ui::homewindow *ui;
    QVector<QPushButton*> mGroup;
    QVector<FanGroupInfo*> mGroups;
    FanGroupInfo *mCurrentGroupInfo;

    CommunicationMode m_communication;
    SerialPortSettings::Settings mSerialPortSettings;


    static homewindow* s_Instance;
};

#endif // HOMEWINDOW_H
