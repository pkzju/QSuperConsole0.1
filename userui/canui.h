#ifndef CANUI_H
#define CANUI_H

#include "canopen/canfestival.h"
#include <QCanBusFrame>

#include <QFrame>
#include "thread/canthread.h"

#include <QThread>


void InitNodes(CO_Data* d, UNS32 id);

void Exit(CO_Data* d, UNS32 id);


namespace Ui {
class CANUi;
}

class CANUi : public QFrame
{
    Q_OBJECT

public:
    explicit CANUi(QWidget *parent = 0);
    ~CANUi();

    static CANUi *getS_Instance();

    void CANopenStart();
    void CANopenStop();

    void resetCAN();

    void mReadResultNetworkDict(CO_Data *d, unsigned char nodeId);

private slots:
    void on_pushButton_Open_clicked();

    void on_pushButton_Close_clicked();

    void on_pushButton_Clear_clicked();

    void messageShow(const QString &s);

    void messageShow(const QCanBusFrame &frame);

    void on_pushButton_Send_clicked();

    void on_pushButton_setNodeState_clicked();

    void on_pushButton_getNodeState_clicked();

    void on_Checkb_CANopen_stateChanged(int arg1);

    void on_pushButton_guardSet_clicked();

    void on_checkBox_sync_toggled(bool checked);

    void on_checkBox_pdoReceive_toggled(bool checked);

    void on_pushButton_dicSet_clicked();

    void on_pushButton_dicRead_clicked();

    void on_pushButton_sdoSet_clicked();

    void on_pushButton_ClearAll_clicked();

    void on_pushButton_sdoRead_clicked();

private:
    Ui::CANUi *ui;
    static CANUi *s_Instance;
    s_BOARD *m_masterBoard;
    CanThread *m_canThread;
    bool m_CANState;
    bool m_CANopenState;

private:
    void initData();
    void initUI();
    void initThread();
    void initConnect();

};


class NMTThread : public QThread
{
    Q_OBJECT
    void run() Q_DECL_OVERRIDE;
public:

    void doPollingState(UNS8 id, int ms);

private:
    UNS8 node_id;
    int timeout_ms;

signals:
    void resultReady(const QString &s);
};

#endif // CANUI_H
