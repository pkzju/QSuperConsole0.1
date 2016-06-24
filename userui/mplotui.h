#ifndef MPLOTUI_H
#define MPLOTUI_H

#include <QWidget>

#include "fanmotor/fanmotor.h"

class QCustomPlot;

namespace Ui {
class MPlotUi;
}

class MPlotUi : public QWidget
{
    Q_OBJECT

public:
    explicit MPlotUi(QWidget *parent = 0);
    ~MPlotUi();
    static MPlotUi *getInstance();
    static void deleteInstance();

public slots:
    void realtimeDataSlot(FanMotorController motorctr);

private:
    Ui::MPlotUi *ui;
    void setupRealtimeDataDemo(QCustomPlot *customPlot);
public:
    static MPlotUi *instance;
    FanMotorController m_motorctr;
};

#endif // MPLOTUI_H
