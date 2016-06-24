/****************************************************************************
**
** Copyright (C) 2016 pkzju
**
** Version	: 0.0.0.1
** Author	:
** 
****************************************************************************/

#include "flywidget.h"
#include<QBitmap>
#include<QApplication>
#include<QMainWindow>
#include<QCursor>


FlyWidget::FlyWidget(QWidget *parent) :
    QWidget(parent),
    settingmenu(NULL)
{
    initData();
    initUI();
    initConnect();
}

void FlyWidget::initData()
{
    m_Pixmap.load(QString(":/images/skin/images/fly.png"));
    resize(m_Pixmap.size());
    setMask(m_Pixmap.mask());
}

void FlyWidget::initUI()
{
    setWindowFlags(Qt::FramelessWindowHint);
    setWindowFlags(Qt::ToolTip);

}

void FlyWidget::initConnect()
{

}

void FlyWidget::setMenu(QMenu *menu)
{
    settingmenu = menu;
}

void FlyWidget::mousePressEvent(QMouseEvent *event)
{
    //left drag  right  close
    if(event->button() == Qt::LeftButton){
        m_CurrentPos = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
    else{
        if(event->button() == Qt::RightButton)
            {
                settingmenu->exec(QCursor::pos());
            }

    }
}

void FlyWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event) //avoid warning
    emit doubleClicked();
}

void FlyWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() && Qt::LeftButton)
    {
        move(event->globalPos() - m_CurrentPos);
        event->accept();
    }
}

void FlyWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.drawPixmap(0, 0, m_Pixmap);
}

void FlyWidget::leaveEvent(QEvent *event)
{
    Q_UNUSED(event)
    setCursor(Qt::ArrowCursor);
}

void FlyWidget::enterEvent(QEvent *event)
{
    Q_UNUSED(event)
    setCursor(Qt::PointingHandCursor);
}

