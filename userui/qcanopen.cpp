#include "qcanopen.h"
#include "ui_qcanopen.h"

QCANopen::QCANopen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QCANopen)
{
    ui->setupUi(this);
}

QCANopen::~QCANopen()
{
    delete ui;
}
