#ifndef CANTHREAD_H
#define CANTHREAD_H


#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>

#include <QCanBusFrame>
#include "canopen/canfestival.h"

#include <QThread>

class CanThread : public QThread
{
    Q_OBJECT

public:
    CanThread(QObject *parent = 0);
    ~CanThread();
    static CanThread* getInstance();
    static void deleteInstance();
    void mStart(bool isCANopen);
    void mStop();
    void run();

signals:
    void message(const QString &s);
    void message(const QCanBusFrame &frame);

private:
    static CanThread* s_instance;
    QMutex m_canMutex;
    bool m_isStopped;
    bool m_isCANopenOpened;

};

#endif // CANTHREAD_H
