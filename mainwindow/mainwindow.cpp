﻿/****************************************************************************
**
** Copyright (C) 2016 pkzju
**
** Version	: 0.0.0.1
** Author	: pkzju
** 
****************************************************************************/

#include "mainwindow.h"
#include "centerwindow.h"
#include "QFramer/futil.h"
#include "thememenu.h"
#include "functionpages/rightfloatwindow.h"
#include <qdebug.h>


MainWindow* MainWindow::instance = nullptr;

MainWindow::MainWindow(QWidget *parent) :
    FMainWindow(parent),
    centerWindow(new CenterWindow), settingMenu (new SettingMenu),
    themeMenu(new ThemeMenu), rightfloatWindow(new RightFloatWindow)
{
    initData();
    initUI();
    initThread();
    initConnect();

}

void MainWindow::initData()
{
}

void MainWindow::initUI()
{
    // set centerWindow
    setCentralWidget(centerWindow);
    centerWindow->getNavgationBar()->setCurrentIndex(0);

    // set setting Menu
    getTitleBar()->getSettingButton()->setMenu(settingMenu);
    getQSystemTrayIcon()->setContextMenu(settingMenu);
    getFlyWidget()->setMenu(settingMenu);

    // set theme Menu
    getTitleBar()->getSkinButton()->setMenu(themeMenu);

    QGridLayout *gdLayout = new QGridLayout();
    rightfloatWindow->setLayout(gdLayout);

    LampOfStatusBar =new QcwIndicatorLamp;
    LampOfStatusBar->setFixedSize(20,20);
    LabelOfStatusBar = new QLabel(QStringLiteral("端口未打开 "));
    RxLabelOfStatusBar = new QLabel(QStringLiteral("接收:00000"));
    TxLabelOfStatusBar = new QLabel(QStringLiteral("发送:00000"));
    OpenOfStatusBar = new QToolButton;
    OpenOfStatusBar->setIcon(QIcon(":/images/skin/images/connect.png"));
    OpenOfStatusBar->setAutoRaise(true);
    OpenOfStatusBar->setIconSize(QSize(25,40));
    CloseOfStatusBar = new QToolButton;
    CloseOfStatusBar->setIcon(QIcon(":/images/skin/images/disconnect.png"));
    CloseOfStatusBar->setIconSize(QSize(25,40));
    OpenOfStatusBar->setAutoRaise(true);

    getStatusBar()->addWidget(LampOfStatusBar);//at the left
    getStatusBar()->addWidget(LabelOfStatusBar);
    getStatusBar()->addWidget(RxLabelOfStatusBar);
    getStatusBar()->addWidget(TxLabelOfStatusBar);
    getStatusBar()->addPermanentWidget(OpenOfStatusBar);//at the right
    getStatusBar()->addPermanentWidget(CloseOfStatusBar);

    connect(OpenOfStatusBar, SIGNAL(clicked()), this, SIGNAL(connectButtonsClicked()));
    connect(CloseOfStatusBar, SIGNAL(clicked()), this, SIGNAL(disconnectButtonsClicked()));

}

void MainWindow::initThread()
{

}

void MainWindow::initConnect()
{
    connect(this, SIGNAL(Hidden()), rightfloatWindow, SLOT(hide()));

}



MainWindow* MainWindow::getInstance()
{
    if(!instance)
    {
        instance = new MainWindow();
    }
    return instance;
}

SettingMenu* MainWindow::getSettingMenu()
{
    return settingMenu;
}

RightFloatWindow* MainWindow::getRightFloatWindow()
{
    return rightfloatWindow;
}


void MainWindow::resizeEvent(QResizeEvent *event)
{
    rightfloatWindow->resize(rightfloatWindow->width(), event->size().height());
    rightfloatWindow->hide();
    QMainWindow::resizeEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    rightfloatWindow->move(x() + width() + 2, y());
    FMainWindow::mouseMoveEvent(event);
}

void MainWindow::showEvent(QHideEvent *event)
{
    rightfloatWindow->setVisible(true);
    event->accept();
}

void MainWindow::hideEvent(QHideEvent *event)
{
    rightfloatWindow->setVisible(false);
    event->accept();
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->y() > getTitleBar()->height())
    {
        rightfloatWindow->setVisible(! rightfloatWindow->isVisible());
    }
    FMainWindow::mouseDoubleClickEvent(event);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Left)
    {
        int index = centerWindow->getNavgationBar()->currentIndex();
        int count = centerWindow->getNavgationBar()->count();
        if(index == 0){
            centerWindow->getNavgationBar()->setCurrentIndex(count - 1);
        }else if(index <= (count - 1) && index > 0){
            centerWindow->getNavgationBar()->setCurrentIndex(index - 1);
        }
    }
    else if(event->key() == Qt::Key_Right)
    {
        int index = centerWindow->getNavgationBar()->currentIndex();
        int count = centerWindow->getNavgationBar()->count();
        if(index == (count - 1)){
            centerWindow->getNavgationBar()->setCurrentIndex(0);
        }else if(index >= 0 && index < (count - 1)){
            centerWindow->getNavgationBar()->setCurrentIndex(index + 1);
        }
    }
    else if(event->key() == Qt::Key_F1)
    {
        delete centerWindow->getNavgationBar()->layout();
        centerWindow->setAlignment(centerWindow->TopCenter);
    }
    else if(event->key() == Qt::Key_Space)
    {
        delete centerWindow->getNavgationBar()->layout();
        int i = 1 + (int)12 * rand() / (RAND_MAX + 1);
        switch (i) {
        case 1:
            centerWindow->setAlignment(centerWindow->TopLeft);
            break;
        case 2:
            centerWindow->setAlignment(centerWindow->TopCenter);
            break;
        case 3:
            centerWindow->setAlignment(centerWindow->TopRight);
            break;
        case 4:
            centerWindow->setAlignment(centerWindow->RightTop);
            break;
        case 5:
            centerWindow->setAlignment(centerWindow->RightCenter);
            break;
        case 6:
            centerWindow->setAlignment(centerWindow->RightBottom);
            break;
        case 7:
            centerWindow->setAlignment(centerWindow->BottomRight);
            break;
        case 8:
            centerWindow->setAlignment(centerWindow->BottomCenter);
            break;
        case 9:
            centerWindow->setAlignment(centerWindow->BottomLeft);
            break;
        case 10:
            centerWindow->setAlignment(centerWindow->LeftBottom);
            break;
        case 11:
            centerWindow->setAlignment(centerWindow->LeftCenter);
            break;
        case 12:
            centerWindow->setAlignment(centerWindow->LeftTop);
            break;
        default:
            break;
        }
    }

    FMainWindow::keyPressEvent(event);
}
