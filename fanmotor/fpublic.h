#ifndef FPUBLIC_H
#define FPUBLIC_H

enum CommunicationMode:char{
    Init,
    Tcp,
    Modbus,
    CANbus
};

struct FanGroupInfo{

    quint16 m_groupID;
    quint16 m_fanNumber;
    CommunicationMode m_com;
};

#endif // FPUBLIC_H
