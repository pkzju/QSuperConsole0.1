#include "fanmotor/qmotor.h"
#include <QObject>

QMotor::QMotor(int address) :
    m_motorAddressLabel{new QLabel(QString("%1").arg(address))},
    m_ratedPowerLCD{new QLCDNumber},
    m_targetPowerLCD{new QLCDNumber},
    m_nowPowerLCD{new QLCDNumber},
    m_ratedSpeedLCD{new QLCDNumber},
    m_speedRefLCD{new QLCDNumber},
    m_speedFbkLCD{new QLCDNumber},
    m_runLamp{new QcwIndicatorLamp},
    m_commLamp{new QcwIndicatorLamp},
    m_message{new QLabel(QString("unknown"))}
{
    m_address = address;
    m_initSetttings = {0};
    m_motorController ={0};
    m_PIPara = {0};
}
QMotor::~QMotor()
{
    delete m_motorAddressLabel;

    delete m_ratedPowerLCD;
    delete m_targetPowerLCD;
    delete m_nowPowerLCD;

    delete m_ratedSpeedLCD;
    delete m_speedRefLCD;
    delete m_speedFbkLCD;

    delete m_runLamp;
    delete m_commLamp;
}
void QMotor::update()
{
    m_ratedPowerLCD->display(m_motorController.m_ratedPower);
    m_targetPowerLCD->display(m_motorController.m_targetpower);
    m_nowPowerLCD->display(m_motorController.m_nowpower);

    m_ratedSpeedLCD->display(m_motorController.m_ratedSpeed);
    m_speedRefLCD->display(m_motorController.m_speedRef);
    m_speedFbkLCD->display(m_motorController.m_speedFbk);

    if(m_motorController.m_runState == FanMotorState::m_run)
        m_runLamp->setLampState(QcwIndicatorLamp::lamp_green);
    else if(m_motorController.m_runState == FanMotorState::m_stop)
        m_runLamp->setLampState(QcwIndicatorLamp::lamp_red);
    else if(m_motorController.m_runState == FanMotorState::m_error){
        m_runLamp->setLampState(QcwIndicatorLamp::lamp_yellow);

        if(m_motorController.m_runError == FanMotorError::m_overCur){
            m_message->setText(QObject::tr("over current"));
        }
        else if(m_motorController.m_runError == FanMotorError::m_overSpd){
            m_message->setText(QObject::tr("over speed"));
        }
        else if(m_motorController.m_runError == FanMotorError::m_stall){
            m_message->setText(QObject::tr("stall"));
        }
        else if(m_motorController.m_runError == FanMotorError::m_lowVolt){
            m_message->setText(QObject::tr("low voltage "));
        }
        else if(m_motorController.m_runError == FanMotorError::m_highVolt){
            m_message->setText(QObject::tr("high voltage "));
        }
        else
            m_message->setText(QObject::tr("no error"));
    }
    else{
        m_runLamp->setLampState(QcwIndicatorLamp::lamp_grey);
    }

    if(m_communicationState == FanCommunicationState::m_connect){
        isMonitor = true;
        m_commLamp->setLampState(QcwIndicatorLamp::lamp_green);
    }
    else if(m_communicationState == FanCommunicationState::m_disconnect){
        isMonitor = false;
        m_commLamp->setLampState(QcwIndicatorLamp::lamp_red);
    }
    else if(m_communicationState == FanCommunicationState::m_comError){
        m_commLamp->setLampState(QcwIndicatorLamp::lamp_yellow);
    }
    else{
        m_commLamp->setLampState(QcwIndicatorLamp::lamp_grey);
    }

}
