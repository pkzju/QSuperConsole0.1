#ifndef SIGLEMOTORFRAME_H
#define SIGLEMOTORFRAME_H

#include <QFrame>

#include "fanmotor/qmotor.h"
#include "qcustomplot/qcustomplot.h"

class QTableWidget;


namespace Ui {
class SigleMotorFrame;
}

class SigleMotorFrame : public QFrame
{
    Q_OBJECT

public:
    explicit SigleMotorFrame(QWidget *parent = 0);
    ~SigleMotorFrame();
    static SigleMotorFrame *getInstance();
    static void deleteInstance();

    void changeMotor(QMotor *motor);

public slots:
    void updateMotorUi(int arg1);
private slots:
    void onStateChange(bool connected);
    void on_pushButton_InitSetF_clicked();
    void on_setPIBtn_clicked();
    void on_readPIBtn_clicked();


protected:
    void hideEvent(QHideEvent *event);
    void closeEvent(QCloseEvent * event);

signals:
    void SigleMotorInitSet(QTableWidget *table, quint16 data);
    void setPI(FanPIParameters &fpi);
    void readPI();

private:
    static SigleMotorFrame *s_Instance;
    Ui::SigleMotorFrame *ui;
    QMotor *m_motor;
    QCPDataMap *m_speedRefData;
    QCPDataMap *m_speedFbkData;
    QCPDataMap *m_idRefData;
    QCPDataMap *m_idFbkData;
    QCPDataMap *m_iqRefData;
    QCPDataMap *m_iqFbkData;

    void setupRealtimeDataDemo(QCustomPlot *customPlot);
    void realtimeDataSlot();
};

#endif // SIGLEMOTORFRAME_H
