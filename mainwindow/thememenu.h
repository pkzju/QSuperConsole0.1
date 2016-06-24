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

#ifndef THEMEMENU_H
#define THEMEMENU_H

#include <QMenu>

class ThemeMenu : public QMenu
{
    Q_OBJECT
private:
    QList<QAction*> actions;
    QList<QString> actionNames;
    QList<QString> actionicons;
    QMap<QString, QAction*> actionMaps;
    QAction* themeAction1;
    QAction* themeAction2;

private:
    void initData();
    void initUI();
    void initConnect();

public:
    explicit ThemeMenu(QWidget *parent = 0);

signals:

public slots:
    void changeTheme1();
    void changeTheme2();
    void changeThemeFromFile();
    void updateCheckedAction(QAction* action);
};

#endif // THEMEMENU_H
