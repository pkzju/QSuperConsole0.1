/****************************************************************************
**
** Copyright (C) 2016 pkzju
**
** Version	: 0.0.0.0
** Author	:
** QQ: 465398889
** Email: dragondjf@gmail.com, ding465398889@163.com, 465398889@qq.com
** 
****************************************************************************/

#include "rightfloatwindow.h"
#include "mainwindow/mainwindow.h"
#include <QGraphicsDropShadowEffect>
#include "mainwindow/settingmenu.h"

RightFloatWindow::RightFloatWindow(QWidget *parent) :
    FMovableWidget(parent)
{
    initData();
    initUI();
    initConnect();

}

void RightFloatWindow::initData()
{
    cwidth = 200;
}

void RightFloatWindow::initUI()
{
    setWindowFlags(Qt::FramelessWindowHint);
    setWindowFlags(Qt::ToolTip);
    setObjectName(QString("RightFloatWindow"));
}

void RightFloatWindow::initConnect()
{

}

void RightFloatWindow::showEvent(QShowEvent *event)
{
    move(MainWindow::getInstance()->x()+ MainWindow::getInstance()->width() + 2,\
         MainWindow::getInstance()->y());
    resize(cwidth, MainWindow::getInstance()->height());
    MainWindow::getInstance()->getSettingMenu()->\
            getActionMaps()[QStringLiteral("显示工具栏")]->setText(QStringLiteral("隐藏工具栏"));
    QWidget::showEvent(event);
}

void RightFloatWindow::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event)
    MainWindow::getInstance()->getSettingMenu()->\
            getActionMaps()[QStringLiteral("显示工具栏")]->setText(QStringLiteral("显示工具栏"));
}

void RightFloatWindow::mouseMoveEvent(QMouseEvent *event)
{
    move(x() + width(), y());
    FMovableWidget::mouseMoveEvent(event);
}

