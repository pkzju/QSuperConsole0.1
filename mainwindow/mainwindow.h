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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "QFramer/fmainwindow.h"
#include "centerwindow.h"
class SettingMenu;
class ThemeMenu;
class RightFloatWindow;
class QcwIndicatorLamp;

#include <QResizeEvent>
#include <QMouseEvent>
#include <QHideEvent>




class MainWindow : public FMainWindow
{
    Q_OBJECT
private:
    void initData();
    void initUI();
    void initThread();
    void initConnect();
protected:
    void mouseMoveEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent * event);
    void hideEvent(QHideEvent * event);
    void showEvent(QHideEvent * event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);

private:
    static MainWindow* instance;
    CenterWindow* centerWindow;
    SettingMenu* settingMenu;
    ThemeMenu* themeMenu;
    RightFloatWindow* rightfloatWindow;
    QcwIndicatorLamp *LampOfStatusBar;

    QToolButton *OpenOfStatusBar;
    QToolButton *CloseOfStatusBar;

signals:
    void connectButtonsClicked();
    void disconnectButtonsClicked();

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    static MainWindow* getInstance();
    RightFloatWindow* getRightFloatWindow();
    SettingMenu* getSettingMenu();
    void setStatusBarLamp(bool isOpen);
    void setStatusBarMessage(QString s);
    void setRxNumber(qint64 num);
    void setTxNumber(qint64 num);
    QToolButton *getOpenButton();
    QToolButton *getCloseButton();
signals:

public slots:



};

#endif // MAINWINDOW_H
