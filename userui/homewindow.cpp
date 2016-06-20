#include "homewindow.h"
#include "ui_homewindow.h"

#include <QPushButton>
#include <QtCore>
#include "mainwindow/mainwindow.h"
#include "userui/fanmotorui.h"
#include "dialogs/siglegroupdialog.h"

#include <QDebug>

extern CO_Data master_Data;

const int gGroupnum = 9; //Number of all groups

homewindow *homewindow::s_Instance = Q_NULLPTR;

homewindow::homewindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::homewindow)
{
    ui->setupUi(this);

    {//Use a container to manage groups
        mGroup.append(ui->pushButton_group1);
        mGroup.append(ui->pushButton_group2);
        mGroup.append(ui->pushButton_group3);
        mGroup.append(ui->pushButton_group4);
        mGroup.append(ui->pushButton_group5);
        mGroup.append(ui->pushButton_group6);
        mGroup.append(ui->pushButton_group7);
        mGroup.append(ui->pushButton_group8);
        mGroup.append(ui->pushButton_group9);

        on_spinBox_groupNum_valueChanged(ui->spinBox_groupNum->value());//Init group number
    }

    for(int i = 0; i < gGroupnum; i++){//All group buttons use one slot fuction
        connect(mGroup.at(i), SIGNAL(clicked(bool)), this, SLOT(button_group_clicked()));
    }


//    FanMotorUi *fanmotorui = FanMotorUi::getS_Instance();
//    connect(this, SIGNAL(connectStateChanged(int)), fanmotorui, SLOT(onConnectStateChanged(int)));


}

homewindow::~homewindow()
{
    delete ui;
}

void homewindow::on_spinBox_groupNum_valueChanged(int arg1)//Add or reduce group
{
    while(mGroups.count() < arg1){
        int _groupID = mGroups.count()+1;
        int _fanMaxNumber = 16;
        int _startAdd = (_groupID-1)*_fanMaxNumber + 1;
        FanGroupInfo *group = new FanGroupInfo{_groupID, _startAdd, _fanMaxNumber};
        group->m_monitorDialog = Q_NULLPTR;
        mGroups.push_back(group);
    }
    while(mGroups.count() > arg1){
        mGroups.pop_back();
    }


    for(int i = arg1; i < gGroupnum; i++)//For reduce group
    {
        mGroup.at(i)->setEnabled(false);
    }
    for(int i = 0; i < arg1; i++)        //For add group
    {
        mGroup.at(i)->setEnabled(true);
    }
}

void homewindow::button_group_clicked()//All group buttons clicked (use one slot fuction)
{
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    for(int i = 0; i < gGroupnum; i++){
        if(btn == mGroup.at(i)){
            mCurrentGroupInfo = mGroups.at(i);
            sigleGroupDialog *mSigleGroupDialog = sigleGroupDialog::getS_Instance();

            mSigleGroupDialog->show(mCurrentGroupInfo);
            break;
        }
    }

}


QStatusBar* homewindow::statusBar()//Get mainwindow's status bar
{
    return MainWindow::getInstance()->getStatusBar();
}


QVector<FanGroupInfo *> *homewindow::groups()
{
    return &mGroups;
}

homewindow *homewindow::getInstance()
{
    if(!s_Instance)
    {
        s_Instance = new homewindow;
    }
    return s_Instance;
}

