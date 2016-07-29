#include "fanhomeframe.h"
#include "ui_fanhomeframe.h"
#include <QButtonGroup>
#include <QTableWidget>

FanHomeFrame::FanHomeFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::FanHomeFrame)
  ,btnGroup(new QButtonGroup)
{
    ui->setupUi(this);

    firstTable = ui->frame1->getTable();
    secondTable = ui->frame2->getTable();

    btnGroup->addButton(reinterpret_cast<QAbstractButton *>(ui->frame1->getGroupBtn()), 1);
    btnGroup->addButton(reinterpret_cast<QAbstractButton *>(ui->frame2->getGroupBtn()), 2);
    connect(btnGroup, SIGNAL(buttonClicked(int)), this, SLOT(enterGroup(int)));

}

FanHomeFrame::~FanHomeFrame()
{
    delete ui;
}

void FanHomeFrame::setGroup(FanGroupInfo *first, FanGroupInfo *second)
{
    firstGroupId = first->m_groupID;
    secondGroupId = second->m_groupID;
    ui->groupBox1->setTitle(tr("Group%1").arg(firstGroupId));
    ui->groupBox2->setTitle(tr("Group%1").arg(secondGroupId));

    if(first){
        QVector<QMotor *> *_motors = &first->m_motors;
        for(int i=0; i < _motors->count(); i++){
            firstTable->setCellWidget(i, 0, _motors->at(i)->m_motorAddressLabel);
            firstTable->setCellWidget(i, 1, _motors->at(i)->m_targetPowerLCD);
            firstTable->setCellWidget(i, 2, _motors->at(i)->m_nowPowerLCD);
            firstTable->setCellWidget(i, 3, _motors->at(i)->m_speedRefLCD);
            firstTable->setCellWidget(i, 4, _motors->at(i)->m_speedFbkLCD);
            firstTable->setCellWidget(i, 5, _motors->at(i)->m_runLamp);
            firstTable->setCellWidget(i, 6, _motors->at(i)->m_commLamp);
            firstTable->setCellWidget(i, 7, _motors->at(i)->m_message);
        }
    }
    if(second){
        QVector<QMotor *> *_motors = &second->m_motors;
        for(int i=0; i < _motors->count(); i++){
            secondTable->setCellWidget(i, 0, _motors->at(i)->m_motorAddressLabel);
            secondTable->setCellWidget(i, 1, _motors->at(i)->m_targetPowerLCD);
            secondTable->setCellWidget(i, 2, _motors->at(i)->m_nowPowerLCD);
            secondTable->setCellWidget(i, 3, _motors->at(i)->m_speedRefLCD);
            secondTable->setCellWidget(i, 4, _motors->at(i)->m_speedFbkLCD);
            secondTable->setCellWidget(i, 5, _motors->at(i)->m_runLamp);
            secondTable->setCellWidget(i, 6, _motors->at(i)->m_commLamp);
            secondTable->setCellWidget(i, 7, _motors->at(i)->m_message);
        }
    }
}

void FanHomeFrame::enterGroup(int id)
{

}
