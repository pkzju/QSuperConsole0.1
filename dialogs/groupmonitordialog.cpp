#include "groupmonitordialog.h"
#include "userui/groupmonitorui.h"

#include "fanmotor/qmotor.h"
#include <QTableWidget>
#include <QDebug>

GroupMonitorDialog::GroupMonitorDialog(QVector<QMotor *> *_motors, QWidget *parent):
    FSubWindown(parent)
{
    initUI();
    if(_motors){
        for(int i=0; i<_motors->count(); i++){
            table()->setCellWidget(i, 0, _motors->at(i)->m_motorAddressLabel);
            table()->setCellWidget(i, 1, _motors->at(i)->m_targetPowerLCD);
            table()->setCellWidget(i, 2, _motors->at(i)->m_nowPowerLCD);
            table()->setCellWidget(i, 3, _motors->at(i)->m_speedRefLCD);
            table()->setCellWidget(i, 4, _motors->at(i)->m_speedFbkLCD);
            table()->setCellWidget(i, 5, _motors->at(i)->m_runLamp);
            table()->setCellWidget(i, 6, _motors->at(i)->m_commLamp);
            table()->setCellWidget(i, 7, _motors->at(i)->m_message);
        }
    }
    m_motorNumber = _motors->count();

}

GroupMonitorDialog::~GroupMonitorDialog()
{

}

void GroupMonitorDialog::initUI()
{
    normalSize = QSize(660, 510);
    getTitleBar()->getTitleLabel()->setText(tr("Group"));
    QVBoxLayout* mainLayout = (QVBoxLayout*)layout();

    mGroupMonitorUi= new GroupMonitorUi;
    mainLayout->addWidget(mGroupMonitorUi);

}
QTableWidget* GroupMonitorDialog::table()
{
    return mGroupMonitorUi->getTable();
}


void GroupMonitorDialog::show(int _groupID, QVector<QMotor*> *_motors)
{
    getTitleBar()->getTitleLabel()->setText(tr("Group%1").arg(_groupID));

    if(!_motors)
        return;

    while(_motors->count() < m_motorNumber){//reduce
        m_motorNumber--;
        table()->removeCellWidget(m_motorNumber, 0);
        table()->removeCellWidget(m_motorNumber, 1);
        table()->removeCellWidget(m_motorNumber, 2);
        table()->removeCellWidget(m_motorNumber, 3);
        table()->removeCellWidget(m_motorNumber, 4);
        table()->removeCellWidget(m_motorNumber, 5);
        table()->removeCellWidget(m_motorNumber, 6);
        table()->removeCellWidget(m_motorNumber, 7);
    }

    while(_motors->count() > m_motorNumber){//add
        table()->setCellWidget(m_motorNumber, 0, _motors->at(m_motorNumber)->m_motorAddressLabel);
        table()->setCellWidget(m_motorNumber, 1, _motors->at(m_motorNumber)->m_targetPowerLCD);
        table()->setCellWidget(m_motorNumber, 2, _motors->at(m_motorNumber)->m_nowPowerLCD);
        table()->setCellWidget(m_motorNumber, 3, _motors->at(m_motorNumber)->m_speedRefLCD);
        table()->setCellWidget(m_motorNumber, 4, _motors->at(m_motorNumber)->m_speedFbkLCD);
        table()->setCellWidget(m_motorNumber, 5, _motors->at(m_motorNumber)->m_runLamp);
        table()->setCellWidget(m_motorNumber, 6, _motors->at(m_motorNumber)->m_commLamp);
        table()->setCellWidget(m_motorNumber, 7, _motors->at(m_motorNumber)->m_message);
        m_motorNumber++;
    }

    if(FSubWindown::isHidden())
        FSubWindown::show();
    else{
        FSubWindown::hide();
        FSubWindown::show();
    }

}


