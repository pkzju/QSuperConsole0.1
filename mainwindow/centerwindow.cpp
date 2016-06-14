/****************************************************************************
**
** Copyright (C) 2014 dragondjf
**
** QFramer is a frame based on Qt5.3, you will be more efficient with it. 
** As an Qter, Qt give us a nice coding experience. With user interactive experience(UE) 
** become more and more important in modern software, deveployers should consider business and UE.
** So, QFramer is born. QFramer's goal is to be a mature solution 
** which you only need to be focus on your business but UE for all Qters.
**
** Version	: 0.2.5.0
** Author	: dragondjf
** Website	: https://github.com/dragondjf
** Project	: https://github.com/dragondjf/QCFramer
** Blog		: http://my.oschina.net/dragondjf/home/?ft=atme
** Wiki		: https://github.com/dragondjf/QCFramer/wiki
** Lincence: LGPL V2
** QQ: 465398889
** Email: dragondjf@gmail.com, ding465398889@163.com, 465398889@qq.com
** 
****************************************************************************/

#include "centerwindow.h"
#include <QHBoxLayout>
#include <qgridlayout.h>
#include "userui/canui.h"
#include "userui/serialportui.h"
#include "userui/mplotui.h"
#include "userui/modbusui.h"
#include "QFramer/ftabwidget.h"
#include "userui/fanmotorui.h"

CenterWindow::CenterWindow(QWidget *parent) :
    FCenterWindow(parent)
{
    initUI();
    initConnect();
}

void CenterWindow::initUI()
{
    qssBuilder = new QssBuilder;
    CANUi *canUi = CANUi::getS_Instance();
    SerialPortUi *serialport = new SerialPortUi;
    MPlotUi *plotUi = MPlotUi::getInstance();
    ModbusUi *modbusui = ModbusUi::getInstance();
    FanMotorUi *fanmotorui = FanMotorUi::getS_Instance();

    addWidget(QStringLiteral("Home"), "Home", fanmotorui);
    addWidget(QStringLiteral("Scope"), "MathPlot", plotUi);
    addWidget(QStringLiteral("Canbus"),"Communication", canUi);
    addWidget(QStringLiteral("Modbus"),"Communication", modbusui);
    addWidget(QStringLiteral("COMPort"),"serialportBtn",serialport);


    setAlignment(TopCenter);
}

void CenterWindow::initConnect()
{

}



