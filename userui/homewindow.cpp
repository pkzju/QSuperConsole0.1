#include "homewindow.h"
#include "ui_homewindow.h"

homewindow::homewindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::homewindow)
{
    ui->setupUi(this);
}

homewindow::~homewindow()
{
    delete ui;
}
