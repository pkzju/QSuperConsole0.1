#include "groupmonitorui.h"
#include "ui_groupmonitorui.h"

GroupMonitorUi::GroupMonitorUi(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::GroupMonitorUi)
{
    ui->setupUi(this);
}

GroupMonitorUi::~GroupMonitorUi()
{
    delete ui;
}

QTableWidget* GroupMonitorUi::getTable()
{
    return ui->table_realtime;
}
