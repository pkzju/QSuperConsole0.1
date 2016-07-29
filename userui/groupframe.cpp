#include "groupframe.h"
#include "ui_groupframe.h"
#include <QTableWidget>
#include <QPushButton>
#include <QLCDNumber>

GroupFrame::GroupFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::GroupFrame)
{
    ui->setupUi(this);
}

GroupFrame::~GroupFrame()
{
    delete ui;
}

void GroupFrame::setTemperature(float t)
{
    ui->lcdNumber_temperature->display(t);
}

void GroupFrame::setHumidity(float h)
{
    ui->lcdNumber_humidity->display(h);
}

QPushButton* GroupFrame::getGroupBtn()
{
    return ui->pushButton_group;
}

QTableWidget* GroupFrame::getTable()
{
    return ui->table_realtime;
}
