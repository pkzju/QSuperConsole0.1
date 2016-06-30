#include "siglemotorframe.h"
#include "ui_siglemotorframe.h"
#include "fanmotorui.h"
#include <QTableWidget>
#include "qcustomplot/qcustomplot.h"

SigleMotorFrame *SigleMotorFrame::s_Instance = Q_NULLPTR;

SigleMotorFrame::SigleMotorFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::SigleMotorFrame)
{
    ui->setupUi(this);

    ui->comboBox_fanType->addItem(QStringLiteral("Type0"), 0);
    ui->comboBox_fanType->addItem(QStringLiteral("Type1"), 1);
    ui->comboBox_fanType->addItem(QStringLiteral("Type2"), 2);
    ui->comboBox_fanType->addItem(QStringLiteral("Type3"), 3);

    ui->comboBox_fanPressure->addItem(QStringLiteral("Pressure0"), 0);
    ui->comboBox_fanPressure->addItem(QStringLiteral("Pressure1"), 1);
    ui->comboBox_fanPressure->addItem(QStringLiteral("Pressure2"), 2);
    ui->comboBox_fanPressure->addItem(QStringLiteral("Pressure3"), 3);

    ui->comboBox_fanreGulation->addItem(QStringLiteral("Regulation0"), 0);
    ui->comboBox_fanreGulation->addItem(QStringLiteral("Regulation1"), 1);
    ui->comboBox_fanreGulation->addItem(QStringLiteral("Regulation2"), 2);
    ui->comboBox_fanreGulation->addItem(QStringLiteral("Regulation2"), 3);

    ui->comboBox_fanControlMode->addItem(QStringLiteral("Mode0"), 0);
    ui->comboBox_fanControlMode->addItem(QStringLiteral("Mode1"), 1);

    setupRealtimeDataDemo(ui->customPlot);

    FanMotorUi *fanmotorui = FanMotorUi::getS_Instance();
    connect(ui->initializeButton, SIGNAL(clicked(bool)), fanmotorui, SLOT(on_initializeButton_clicked()));
    connect(ui->pushButton_startMotor, SIGNAL(clicked(bool)), fanmotorui, SLOT(onMotorStartButtonClicked()));
    connect(ui->pushButton_stopMotor, SIGNAL(clicked(bool)), fanmotorui, SLOT(onMotorStopButtonClicked()));
    connect(ui->pushButton_InitRead, SIGNAL(clicked(bool)), fanmotorui, SLOT(on_pushButton_InitRead_clicked()));
    connect(this, SIGNAL(SigleMotorInitSet(QTableWidget*,quint16)), fanmotorui, SLOT(onSigleMotorInitSetClicked(QTableWidget*,quint16)));
    connect(fanmotorui, SIGNAL(updateSigleMotor(int)), this, SLOT(updateMotorUi(int)));
    connect(fanmotorui, SIGNAL(updateSigleMotorState(bool)), this, SLOT(onStateChange(bool)));
    connect(ui->checkBox, SIGNAL(stateChanged(int)), fanmotorui, SLOT(monitorSigleStateChange(int)));
    connect(this, SIGNAL(setPI()), fanmotorui, SLOT(onSetPI()));
    connect(this, SIGNAL(readPI()), fanmotorui, SLOT(onReadPI()));



}

SigleMotorFrame::~SigleMotorFrame()
{
    s_Instance = Q_NULLPTR;
    delete ui;
}

SigleMotorFrame *SigleMotorFrame::getInstance()
{
    if(!s_Instance)
    {
        s_Instance = new SigleMotorFrame;
    }
    return s_Instance;
}

void SigleMotorFrame::deleteInstance()
{
    if(s_Instance)
        s_Instance->deleteLater();
}

void SigleMotorFrame::changeMotor(QMotor *motor)
{
    m_motor = motor;

    updateMotorUi(0);
    updateMotorUi(1);
    updateMotorUi(2);

    ui->lcdNumber_fanID->display(motor->m_address);
//    ui->checkBox->setChecked(motor->isMonitor);
}

void SigleMotorFrame::updateMotorUi(int arg1)
{
    if(this->isHidden())
        return;
    //Init parameter update
    if(arg1 == 0){
        //Update data to settings table
        QAbstractItemModel *__model = ui->table_settings->model();
        quint16 *__buffPtr = (quint16 *)&m_motor->m_initSetttings;
        for(unsigned char i = 0; i<3; i++){
            for(unsigned char j = 1; j<5; j++){
                __model->setData(__model->index(i,j), (double)(*__buffPtr) * 0.01);
                __buffPtr++;
            }
        }

        int _fanType = (*__buffPtr >> 12) & 0x000f;
        ui->comboBox_fanType->setCurrentIndex(_fanType);
        int _fanPressure = (*__buffPtr >> 10) & 0x0003;
        ui->comboBox_fanPressure->setCurrentIndex(_fanPressure);
        int _fanreGulation = (*__buffPtr >> 8) & 0x0003;
        ui->comboBox_fanreGulation->setCurrentIndex(_fanreGulation);
        int _fanControlMode = (*__buffPtr >> 7) & 0x0001;
        ui->comboBox_fanControlMode->setCurrentIndex(_fanControlMode);
    }

    //Monitor data update
    if(arg1 == 1){
        ui->lcdNumber_targetPower->display(m_motor->m_motorController.m_targetpower);
        ui->lcdNumber_realTimePower->display(m_motor->m_motorController.m_nowpower);

        ui->lcdNumber_targetSpeed->display(m_motor->m_motorController.m_speedRef);
        ui->lcdNumber_realTimeSpeed->display(m_motor->m_motorController.m_speedFbk);

        ui->lcdNumber_targetId->display(m_motor->m_motorController.m_idRef);
        ui->lcdNumber_realTimeId->display(m_motor->m_motorController.m_idFbk);

        ui->lcdNumber_targetIq->display(m_motor->m_motorController.m_iqRef);
        ui->lcdNumber_realTimeIq->display(m_motor->m_motorController.m_iqFbk);

        if(ui->checkBox->isChecked())
            realtimeDataSlot();

    }

    //Initialize data update
    if(arg1 == 2){
        ui->lcdNumber_ratedPower->display(m_motor->m_motorController.m_ratedPower);
        ui->lcdNumber_ratedSpeed->display(m_motor->m_motorController.m_ratedSpeed);
    }

    if(m_motor->m_motorController.m_runState == FanMotorState::m_run)
        ui->runLamp->setLampState(QcwIndicatorLamp::lamp_green);
    else if(m_motor->m_motorController.m_runState == FanMotorState::m_stop)
        ui->runLamp->setLampState(QcwIndicatorLamp::lamp_red);
    else if(m_motor->m_motorController.m_runState == FanMotorState::m_error){
        ui->runLamp->setLampState(QcwIndicatorLamp::lamp_yellow);
    }
    else{
        ui->runLamp->setLampState(QcwIndicatorLamp::lamp_grey);
    }

    if(m_motor->m_communicationState == FanCommunicationState::m_connect)
        ui->comLamp->setLampState(QcwIndicatorLamp::lamp_green);
    else if(m_motor->m_communicationState == FanCommunicationState::m_disconnect)
        ui->comLamp->setLampState(QcwIndicatorLamp::lamp_red);
    else if(m_motor->m_communicationState == FanCommunicationState::m_comError)
        ui->comLamp->setLampState(QcwIndicatorLamp::lamp_yellow);
    else
        ui->comLamp->setLampState(QcwIndicatorLamp::lamp_grey);

    if(arg1 == 3){//Read PI ready
        ui->SpinBox_speedPRW->setValue((double) m_motor->m_PIPara.m_speedKp * 0.001);
        ui->SpinBox_speedIRW->setValue((double) m_motor->m_PIPara.m_speedKi * 0.001);
        ui->SpinBox_idPRW->setValue((double) m_motor->m_PIPara.m_idKp * 0.001);
        ui->SpinBox_idIRW->setValue((double) m_motor->m_PIPara.m_idKi * 0.001);
        ui->SpinBox_iqPRW->setValue((double) m_motor->m_PIPara.m_iqKp * 0.001);
        ui->SpinBox_iqIRW->setValue((double) m_motor->m_PIPara.m_iqKi * 0.001);
    }

}

void SigleMotorFrame::onStateChange(bool connected)
{
    ui->checkBox->setEnabled(connected);
    ui->initializeButton->setEnabled(connected);
    ui->pushButton_InitRead->setEnabled(connected);
    ui->pushButton_InitSetF->setEnabled(connected);
    ui->pushButton_startMotor->setEnabled(connected);
    ui->pushButton_stopMotor->setEnabled(connected);
    ui->setPIBtn->setEnabled(connected);
    ui->readPIBtn->setEnabled(connected);
}

void SigleMotorFrame::on_pushButton_InitSetF_clicked()
{
    int _fanType = ui->comboBox_fanType->currentIndex();
    int _fanPressure = ui->comboBox_fanPressure->currentIndex();
    int _fanreGulation = ui->comboBox_fanreGulation->currentIndex();
    int _fanControlMode = ui->comboBox_fanControlMode->currentIndex();
    int data = (_fanType << 12) + (_fanPressure << 10) + (_fanreGulation << 8) + (_fanControlMode << 7);
    emit SigleMotorInitSet(ui->table_settings, data);
}



void SigleMotorFrame::on_setPIBtn_clicked()
{
    m_motor->m_PIPara.m_speedKp = (quint16)(ui->SpinBox_speedPRW->value() * 1000);
    m_motor->m_PIPara.m_speedKi = (quint16)(ui->SpinBox_speedIRW->value() * 1000);
    m_motor->m_PIPara.m_idKp = (quint16)(ui->SpinBox_idPRW->value() * 1000);
    m_motor->m_PIPara.m_idKi = (quint16)(ui->SpinBox_idIRW->value() * 1000);
    m_motor->m_PIPara.m_iqKp = (quint16)(ui->SpinBox_iqPRW->value() * 1000);
    m_motor->m_PIPara.m_iqKi = (quint16)(ui->SpinBox_iqIRW->value() * 1000);
    emit setPI();
}

void SigleMotorFrame::on_readPIBtn_clicked()
{

    emit readPI();
}

void SigleMotorFrame::setupRealtimeDataDemo(QCustomPlot *customPlot)
{
#if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
    QMessageBox::critical(this, "", "You're using Qt < 4.7, the realtime data demo needs functions that are available with Qt 4.7 to work properly");
#endif

    // include this section to fully disable antialiasing for higher performance:
    customPlot->setNotAntialiasedElements(QCP::aeAll);
    QFont font;
    font.setStyleStrategy(QFont::NoAntialias);
    customPlot->xAxis->setTickLabelFont(font);
    customPlot->yAxis->setTickLabelFont(font);
    customPlot->legend->setFont(font);


    customPlot->addGraph(); // blue line
    customPlot->graph(0)->setPen(QPen(Qt::blue));
    customPlot->graph(0)->setName("Set");
    customPlot->addGraph(); // red line
    customPlot->graph(1)->setPen(QPen(Qt::red));
    customPlot->graph(1)->setName("Feedback");
//    customPlot->addGraph(); // green line
//    customPlot->graph(2)->setPen(QPen(Qt::green));


    customPlot->addGraph(); // blue dot
    customPlot->graph(2)->setPen(QPen(Qt::blue));
    customPlot->graph(2)->setLineStyle(QCPGraph::lsNone);
    customPlot->graph(2)->setScatterStyle(QCPScatterStyle::ssDisc);
    customPlot->addGraph(); // red dot
    customPlot->graph(3)->setPen(QPen(Qt::red));
    customPlot->graph(3)->setLineStyle(QCPGraph::lsNone);
    customPlot->graph(3)->setScatterStyle(QCPScatterStyle::ssDisc);
//    customPlot->addGraph(); // green dot
//    customPlot->graph(5)->setPen(QPen(Qt::green));
//    customPlot->graph(5)->setLineStyle(QCPGraph::lsNone);
//    customPlot->graph(5)->setScatterStyle(QCPScatterStyle::ssDisc);

    customPlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    customPlot->xAxis->setDateTimeFormat("ss");
    customPlot->xAxis->setAutoTickStep(false);
    customPlot->xAxis->setTickStep(1);
    //  customPlot->axisRect()->setupFullAxesBox();

    // setup legend:
    customPlot->legend->setVisible(true);
    customPlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop|Qt::AlignLeft);
    customPlot->legend->setBrush(QColor(80, 80, 80, 200));
    QPen legendPen;
    legendPen.setColor(QColor(130, 130, 130, 200));
    customPlot->legend->setBorderPen(legendPen);
    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    customPlot->legend->setTextColor(Qt::white);
    customPlot->legend->removeItem(customPlot->legend->itemCount()-1);
    customPlot->legend->removeItem(customPlot->legend->itemCount()-1);
//    customPlot->legend->removeItem(customPlot->legend->itemCount()-1);

    // set some pens, brushes and backgrounds:
    customPlot->xAxis->setBasePen(QPen(Qt::white, 1));
    customPlot->yAxis->setBasePen(QPen(Qt::white, 1));
    customPlot->xAxis->setTickPen(QPen(Qt::white, 1));
    customPlot->yAxis->setTickPen(QPen(Qt::white, 1));
    customPlot->xAxis->setSubTickPen(QPen(Qt::white, 1));
    customPlot->yAxis->setSubTickPen(QPen(Qt::white, 1));
    customPlot->xAxis->setTickLabelColor(Qt::white);
    customPlot->yAxis->setTickLabelColor(Qt::white);
    customPlot->xAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
    customPlot->yAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
    customPlot->xAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
    customPlot->yAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
    customPlot->xAxis->grid()->setSubGridVisible(true);
    customPlot->yAxis->grid()->setSubGridVisible(true);
    customPlot->xAxis->grid()->setZeroLinePen(Qt::NoPen);
    customPlot->yAxis->grid()->setZeroLinePen(Qt::NoPen);
    customPlot->xAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);
    customPlot->yAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);
    QLinearGradient plotGradient;
    plotGradient.setStart(0, 0);
    plotGradient.setFinalStop(0, 350);
    plotGradient.setColorAt(0, QColor(80, 80, 80));
    plotGradient.setColorAt(1, QColor(50, 50, 50));
    customPlot->setBackground(plotGradient);
    QLinearGradient axisRectGradient;
    axisRectGradient.setStart(0, 0);
    axisRectGradient.setFinalStop(0, 350);
    axisRectGradient.setColorAt(0, QColor(80, 80, 80));
    axisRectGradient.setColorAt(1, QColor(30, 30, 30));
    customPlot->axisRect()->setBackground(axisRectGradient);

    for(int i=0; i <100; i++){
        double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
        ui->customPlot->graph(0)->addData(key,0);
        ui->customPlot->graph(1)->addData(key,0);
    }
    ui->customPlot->xAxis->setRange(ui->customPlot->graph(0)->data()->firstKey(), ui->customPlot->graph(0)->data()->lastKey()+0.1);
    // make left and bottom axes transfer their ranges to right and top axes:
    connect(customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->yAxis2, SLOT(setRange(QCPRange)));

}

void SigleMotorFrame::realtimeDataSlot()
{

    double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;

    // add data to lines:
    ui->customPlot->graph(0)->addData(key, m_motor->m_motorController.m_speedRef);
    ui->customPlot->graph(1)->addData(key, m_motor->m_motorController.m_speedFbk);
//    ui->customPlot->graph(2)->addData(key, 0);
    // set data of dots:
    ui->customPlot->graph(2)->clearData();
    ui->customPlot->graph(2)->addData(key, m_motor->m_motorController.m_speedRef);
    ui->customPlot->graph(3)->clearData();
    ui->customPlot->graph(3)->addData(key, m_motor->m_motorController.m_speedFbk);
//    ui->customPlot->graph(5)->clearData();
//    ui->customPlot->graph(5)->addData(key, 0);

    // remove data of lines that's outside visible range:
    if(ui->customPlot->graph(0)->data()->count()>100)
        ui->customPlot->graph(0)->removeData(ui->customPlot->graph(0)->data()->firstKey());
    if(ui->customPlot->graph(1)->data()->count()>100)
        ui->customPlot->graph(1)->removeData(ui->customPlot->graph(1)->data()->firstKey());
//    if(ui->customPlot->graph(2)->data()->count()>100)
//        ui->customPlot->graph(2)->removeData(ui->customPlot->graph(2)->data()->begin().key());
    // rescale value (vertical) axis to fit the current data:
    ui->customPlot->graph(0)->rescaleValueAxis(false);
    ui->customPlot->graph(1)->rescaleValueAxis(true);
//    ui->customPlot->graph(2)->rescaleValueAxis(true);

    ui->customPlot->xAxis->setRange(ui->customPlot->graph(0)->data()->firstKey(), ui->customPlot->graph(0)->data()->lastKey()+0.1);

    ui->customPlot->replot();

}
