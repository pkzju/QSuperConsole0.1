/****************************************************************************
**
** Copyright (C) 2016 pkzju
**
**
** Version	: 0.1.1.0
** Author	: pkzju
** Website	: https://github.com/pkzju
** Project	: https://github.com/pkzju/QSuperConsole
** 
****************************************************************************/

#ifndef QCWINDCCATORLAMP_H
#define QCWINDCCATORLAMP_H

#include <QWidget>


class QColor;

class Q_DECL_EXPORT QcwIndicatorLamp: public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool m_alarm READ isAlarm WRITE setAlarm)

public:
    enum LampState:char
    {
        lamp_grey  ,
        lamp_red   ,
        lamp_green ,
        lamp_yellow,
        lamp_blue  ,
        lamp_alarm
    };


public:    
    explicit QcwIndicatorLamp(QWidget *parent = 0);
    bool isAlarm() const {return m_alarm;}


    QcwIndicatorLamp::LampState getLampState();

Q_SIGNALS:

public Q_SLOTS:
    void setAlarm(bool);
    void setLampState(QcwIndicatorLamp::LampState s);

protected:
    void paintEvent(QPaintEvent *);

private:
    bool m_alarm;
    LampState lamp_state;
};

#endif
