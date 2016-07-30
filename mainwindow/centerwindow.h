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
#include <QVBoxLayout>

#include "QFramer/fcenterwindow.h"

#include "fanmotor/fpublic.h"

class CenterWindow : public FCenterWindow
{
    Q_OBJECT

public:
    explicit CenterWindow(QWidget *parent = 0);
    ~CenterWindow();
    void initData();
    void initUI();
    void initConnect();
    QVector<FanGroupInfo*> &getGroups();

signals:

public slots:
//    void cloudAntimation();

private:
    QVector<FanGroupInfo*> motorGroups;
};

#endif // CENTERWINDOW_H
