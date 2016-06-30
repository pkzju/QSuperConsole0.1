#include "allgroupframe.h"
#include "ui_allgroupframe.h"

AllGroupFrame::AllGroupFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::AllGroupFrame)
{
    ui->setupUi(this);
}

AllGroupFrame::~AllGroupFrame()
{
    delete ui;
}
