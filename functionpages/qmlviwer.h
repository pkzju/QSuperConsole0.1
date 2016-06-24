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

#ifndef QMLVIWER_H
#define QMLVIWER_H

#include <QWidget>
#include <QLineEdit>
#include <QToolButton>
#include "basequickwidget.h"
class QmlViwer : public QWidget
{
    Q_OBJECT
public:
    BaseQuickWidget* viewer;
    QToolButton* loadQmlButton;
    QLineEdit* qmlUrlEdit;
public:
    explicit QmlViwer(QWidget *parent = 0);
    void initData();
    void initUI();
    void initConnect();

signals:

public slots:
    void getQmlUrl();
    void loadQml();
};

#endif // QMLVIWER_H
