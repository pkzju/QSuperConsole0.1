#ifndef SERIALPORTUI_H
#define SERIALPORTUI_H

#include <QDialog>

#include "thread/serialportthread.h"


namespace Ui {
class SerialPortUi;
}

class SerialPortUi : public QDialog
{
    Q_OBJECT
private:
    void initData();
    void initUI();
    void initThread();
    void initConnect();

public:
    explicit SerialPortUi(QWidget *parent = 0);
    ~SerialPortUi();


private slots:

    void radioButton_clicked();
    void messageShow(const QString &s);

private:
    Ui::SerialPortUi *ui;
    SerialPortThread *mySerialPortThread;
};

#endif // SERIALPORTUI_H
