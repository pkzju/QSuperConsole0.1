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

#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H
#include "QFramer/fbasedialog.h"

#include"userui/serialportsettingsdialog.h"

class SettingDialog : public FBaseDialog
{
    Q_OBJECT
public:
    explicit SettingDialog(QWidget *parent = 0);
    void initUI();
    void initConnect();

signals:

public slots:

private:

};

#endif // SETTINGDIALOG_H
