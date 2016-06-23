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

#ifndef CENTERWINDOW_H
#define CENTERWINDOW_H

#include "QFramer/fcenterwindow.h"
#include "functionpages/qssbuilder.h"


#include <QVBoxLayout>


class CenterWindow : public FCenterWindow
{
    Q_OBJECT

private:


public:
    explicit CenterWindow(QWidget *parent = 0);
    ~CenterWindow();
    void initUI();
    void initConnect();



signals:

public slots:
//    void cloudAntimation();
};

#endif // CENTERWINDOW_H
