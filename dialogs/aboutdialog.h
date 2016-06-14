/****************************************************************************
**
** Copyright (C) 2016 pkzju
**
**
** Version	: 0.1.1.0
** Author	: pkzju
** Website	: https://github.com/pkzju
** Project	: https://github.com/pkzju/QSuperConsole
** 
****************************************************************************/

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include "QFramer/fbasedialog.h"
#include <QShowEvent>


class AboutDialog : public FBaseDialog
{
    Q_OBJECT
private:
    QPushButton* enterButton;
protected:
     void showEvent(QShowEvent * event);
public:
    explicit AboutDialog(QWidget *parent = 0);
    void initUI();
    void initConnect();
signals:

public slots:
    void animationClose();
};

#endif // ABOUTDIALOG_H
