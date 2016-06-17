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

#ifndef SIGLEGROUPDIALOG_H
#define SIGLEGROUPDIALOG_H

#include "QFramer/fsubwindown.h"



class sigleGroupDialog : public FSubWindown
{
    Q_OBJECT
public:
    explicit sigleGroupDialog(QWidget *parent = 0);
    ~sigleGroupDialog();
    void initUI();
    void initConnect();
    static sigleGroupDialog *getS_Instance();
signals:

public slots:

private:
    static sigleGroupDialog *s_Instance;

};

#endif // SIGLEGROUPDIALOG_H
