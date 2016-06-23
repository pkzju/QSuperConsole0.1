#include "canthread.h"
#include <QDebug>

//Q_DECLARE_METATYPE(QCanBusFrame)

CanThread *CanThread::s_instance = Q_NULLPTR;

CanThread::CanThread(QObject *parent)
    : QThread(parent)
{
    qRegisterMetaType<QCanBusFrame>("QCanBusFrame");
}

CanThread::~CanThread()
{
    s_instance = Q_NULLPTR;
    wait();
}


CanThread* CanThread::getInstance()
{
    if(!s_instance)
    {
        s_instance = new CanThread();
    }
    return s_instance;
}

void CanThread::deleteInstance()
{
    if(s_instance)
        s_instance->deleteLater();
}

void CanThread::mStart(bool isCANopen)
{
    m_canMutex.lock();
    m_isStopped = false;
    m_isCANopenOpened = isCANopen;
    m_canMutex.unlock();

    if (!this->isRunning())
        this->start();
}

void CanThread::mStop()
{
    m_canMutex.lock();
    m_isStopped = true;
    m_isCANopenOpened = false;
    m_canMutex.unlock();
}


void CanThread::run()
{
    bool isStopped = true;
    bool isCANopen = true;
    m_canMutex.lock();
    isStopped = m_isStopped;
    isCANopen = m_isCANopenOpened;
    m_canMutex.unlock();

    qDebug("thread running");

    Message m{0};
    QCanBusFrame frame;
    QByteArray payload;
    while(!isStopped)
    {
        int ret=0;

        ret = usbCanReceive(0, &m);

        if(ret>0){

            if(isCANopen)
                canDispatch(&master_Data, &m);

            frame.setFrameId(m.cob_id);

            payload.clear();
            for(quint8 i = 0 ; i < m.len ; i++){
                payload.append(m.data[i]);
            }
            frame.setPayload(payload);
            message(frame);
        }

        m_canMutex.lock();
        isStopped = m_isStopped;
        isCANopen = m_isCANopenOpened;
        m_canMutex.unlock();
    }

    qDebug("thread exit");
}
