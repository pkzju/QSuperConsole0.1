#include "fsubwindown.h"


#include <QPropertyAnimation>
#include <QDesktopWidget>
#include <QApplication>
#include <QGraphicsDropShadowEffect>
#include <QPushButton>


FSubWindown::FSubWindown(QWidget *parent) :
    QDialog(parent)
{
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_DeleteOnClose);
    initData();
    initUI();
    initConnect();
}

void FSubWindown::initData()
{

}

void FSubWindown::initUI()
{
    resize(0, 0);
    titlebar = new FTitleBar;
    titlebar->setSettingButtonVisible(false);
    titlebar->setSkinButtonVisible(false);
    titlebar->setFixButtonVisible(false);
//    titlebar->setMinButtonVisible(false);
    titlebar->setMaxButtonVisible(false);

    QVBoxLayout* mainLayout = new QVBoxLayout;

    mLayout = new QVBoxLayout;

    QWidget *widget = new QWidget;
    widget->setObjectName(tr("needBorder"));

    widget->setLayout(mLayout);
    mLayout->addWidget(titlebar);
    mLayout->setContentsMargins(0, 0, 0, 0);
    mLayout->setSpacing(0);
    mainLayout->addWidget(widget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    setLayout(mainLayout);
}

QLayout *FSubWindown::layout()
{
    return mLayout;
}

void FSubWindown::initConnect()
{
    connect(titlebar->getMinButton(), SIGNAL(clicked()), this, SLOT(showMinimized()));
    connect(titlebar->getCloseButton(), SIGNAL(clicked()), this, SLOT(hide()));
}

FTitleBar* FSubWindown::getTitleBar()
{
    return titlebar;
}

void FSubWindown::mousePressEvent(QMouseEvent *e)
{
    if(e->button() & Qt::LeftButton)
    {
        dragPosition = e->globalPos() - frameGeometry().topLeft();
    }
    e->accept();
}

void FSubWindown::mouseReleaseEvent(QMouseEvent *e)
{
    e->accept();
}

void FSubWindown::mouseMoveEvent(QMouseEvent *e)
{
    if(e->y() < height() - 30){
        move(e->globalPos() - dragPosition);
        e->accept();
    }
    else{
        e->ignore();
    }

}

void FSubWindown::showEvent(QShowEvent *event)
{

    QDesktopWidget* desktopWidget = QApplication::desktop();
    QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry");
    animation->setDuration(200);
    int w = desktopWidget->availableGeometry().width();
    int h = desktopWidget->availableGeometry().height();
    animation->setStartValue(QRect(w * 0.5, h*0.5, 0, 0));
    animation->setEndValue(QRect(w /2 - normalSize.width()/2,
                                 h / 2 - normalSize.height() / 2,
                                 normalSize.width(), normalSize.height()));
    animation->start();
    QWidget::showEvent(event);
}

void FSubWindown::closeEvent(QCloseEvent *event)
{

    QWidget::closeEvent(event);
}

void FSubWindown::animationClose()
{
    QRect noraml = geometry();
    QRect closeRect = QRect(noraml.x(), noraml.y() + noraml.height()/2, noraml.width(), 0);
    QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry");
    connect(animation, SIGNAL(finished()), this, SLOT(close()));
    animation->setDuration(100);
    animation->setStartValue(noraml);
    animation->setEndValue(closeRect);
    animation->start();
}
