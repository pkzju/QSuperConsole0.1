#ifndef FPUBLIC_H
#define FPUBLIC_H

#include "qmotor.h"
#include "dialogs/groupmonitordialog.h"

enum CommunicationMode:char{
    Init,
    Tcp,
    Modbus,
    CANbus
};

class FanGroupInfo{

public:
    quint16 m_groupID;
    quint16 m_startAddress;
    quint16 m_fanMaxNumber;
    QVector<QMotor *> m_motors;

    bool isMonitor;
    GroupMonitorDialog *m_monitorDialog;

};

#endif // FPUBLIC_H
