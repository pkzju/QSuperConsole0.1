

#include "aboutdialog.h"
#include "QFramer/fbasepushbutton.h"
#include "QFramer/fshadowlabel.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QDesktopWidget>
#include <QApplication>


AboutDialog::AboutDialog(QWidget *parent) :
    FBaseDialog(parent)
{
    initUI();
    initConnect();
}

void AboutDialog::initUI()
{
    normalSize = QSize(400, 300);

    FShadowLabel *logoLabel = new FShadowLabel(tr("QSuperConsole"));
    logoLabel->setObjectName(QString("logoLabel"));

    FShadowLabel *infoLabel = new FShadowLabel(tr("Faster Easier Stronger Prettier"));
    infoLabel->setObjectName(QString("content"));

    FShadowLabel *lincenceLabel = new FShadowLabel(tr("lincence: LGPL v3"), this);
    lincenceLabel->setObjectName(QString("content"));

    FShadowLabel *authorLabel = new FShadowLabel(tr("author:pkzju"));
    authorLabel->setObjectName(QString("content"));


    FShadowLabel *versionLabel = new FShadowLabel(tr("version:0.1.0.0"));
    versionLabel->setObjectName(QString("content"));


    enterButton = new FBasePushButton();
    enterButton->setText(tr("Close"));
    QHBoxLayout* controlLayout = new QHBoxLayout;
    controlLayout->addStretch();
    controlLayout->addWidget(enterButton);
    controlLayout->addSpacing(10);

    QVBoxLayout* mainLayout = (QVBoxLayout*)layout();
    mainLayout->addSpacing(20);
    mainLayout->addWidget(logoLabel);
    mainLayout->addWidget(infoLabel);
    mainLayout->addSpacing(30);
    mainLayout->addWidget(lincenceLabel);
    mainLayout->addWidget(authorLabel);
    mainLayout->addWidget(versionLabel);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(controlLayout);
    mainLayout->addSpacing(10);



}

void AboutDialog::initConnect()
{
//    BaseDialog::initConnect();
    connect(enterButton, SIGNAL(clicked()), this, SLOT(animationClose()));
}

void AboutDialog::showEvent(QShowEvent *event)
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry");

    QDesktopWidget* desktopWidget = QApplication::desktop();
    int x = (desktopWidget->availableGeometry().width() - normalSize.width()) / 2;
    int y = (desktopWidget->availableGeometry().height() - normalSize.height()) / 2;

    connect(animation, SIGNAL(finished()), animation, SLOT(deleteLater()));
    animation->setDuration(1500);
    animation->setStartValue(QRect(x, 0, normalSize.width(), normalSize.height()));
    animation->setEndValue(QRect(x, y, normalSize.width(), normalSize.height()));
    animation->setEasingCurve(QEasingCurve::OutElastic);
    animation->start();
    QDialog::showEvent(event);
}

void AboutDialog::animationClose()
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "windowOpacity");
    connect(animation, SIGNAL(finished()), this, SLOT(deleteLater()));
    connect(animation, SIGNAL(finished()), this, SLOT(close()));
    animation->setDuration(1500);
    animation->setStartValue(1);
    animation->setEndValue(0);
    animation->start();
}
