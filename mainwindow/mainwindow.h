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

class QModbusClient;

#include <QResizeEvent>
#include <QMouseEvent>
#include <QHideEvent>
#include <QModbusClient>
#include <QModbusDataUnit>



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
    CenterWindow* centerWindow = Q_NULLPTR;
    SettingMenu* settingMenu = Q_NULLPTR;
    ThemeMenu* themeMenu = Q_NULLPTR;
    RightFloatWindow* rightfloatWindow = Q_NULLPTR;
    QcwIndicatorLamp *LampOfStatusBar = Q_NULLPTR;

    QToolButton *OpenOfStatusBar = Q_NULLPTR;
    QToolButton *CloseOfStatusBar = Q_NULLPTR;
    QModbusClient *modbusRtuDevice = Q_NULLPTR;//!< Modbus RTU device


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
    QModbusClient *getRtuDevice();
    bool sendRtuWriteRequest(QModbusDataUnit unit, quint16 serverAddress);
    QModbusReply *sendRtuReadRequest(const QModbusDataUnit &read, int serverAddress);
signals:

public slots:

    void onConnectButtonClicked();
    void onDisconnectButtonClicked();
    void onStateChanged(int state);
};

#endif // MAINWINDOW_H
