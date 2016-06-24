#include "qcw_indicatorlamp.h"
#include <QPainter>

QcwIndicatorLamp::QcwIndicatorLamp(QWidget *parent): QWidget(parent)
{
    m_alarm = false;
    lamp_state = QcwIndicatorLamp::lamp_grey;
    resize(100, 100);
}

void QcwIndicatorLamp::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing); 	/* 使用反锯齿(如果可用) */
	painter.translate(width() / 2, height() / 2);	/* 坐标变换为窗体中心 */
	int side = qMin(width(), height());
	painter.scale(side / 200.0, side / 200.0);		/* 比例缩放 */
	painter.setPen(Qt::NoPen);
	
    int radius = 100;
 
    /* 外边框 */
    QLinearGradient lg1(0, -radius, 0, radius);
    lg1.setColorAt(0, QColor(255, 255, 255));
    lg1.setColorAt(1, QColor(166, 166, 166));
    painter.setBrush(lg1);
    painter.drawEllipse(-radius, -radius, radius << 1, radius << 1);

    /* 内边框 */
    radius -= 13;
    QLinearGradient lg2(0, -radius, 0, radius);
    lg2.setColorAt(0, QColor(166, 166, 166));
    lg2.setColorAt(1, QColor(255, 255, 255));
    painter.setBrush(lg2);
    painter.drawEllipse(-radius, -radius, radius << 1, radius << 1);

	/* 内部指示颜色 */
    radius -= 4;
    QRadialGradient rg(0, 0, radius);
    if(lamp_state == QcwIndicatorLamp::lamp_red)
    {
        rg.setColorAt(0, QColor(245, 0, 0));
        rg.setColorAt(0.6, QColor(210, 0, 0));
        rg.setColorAt(1, QColor(166, 0, 0));
    }
    else if(lamp_state == QcwIndicatorLamp::lamp_green)
    {
        rg.setColorAt(0, QColor(0, 245, 0));
        rg.setColorAt(0.6, QColor(0, 210, 0));
        rg.setColorAt(1, QColor(0, 166, 0));
    }
    else if(lamp_state == QcwIndicatorLamp::lamp_yellow)
    {
        rg.setColorAt(0, QColor(245, 245, 0));
        rg.setColorAt(0.6, QColor(210, 210, 0));
        rg.setColorAt(1, QColor(166, 166, 0));
    }
    else if(lamp_state == QcwIndicatorLamp::lamp_blue)
    {
        rg.setColorAt(0, QColor(0, 0, 245));
        rg.setColorAt(0.6, QColor(0, 0, 210));
        rg.setColorAt(1, QColor(0, 0, 166));
    }
    else if(lamp_state == QcwIndicatorLamp::lamp_grey)
    {
        rg.setColorAt(0, QColor(211, 211, 211));
        rg.setColorAt(0.6, QColor(200, 200, 200));
        rg.setColorAt(1, QColor(190, 190, 190));
    }
    else
    {
        if (m_alarm)
        {
            rg.setColorAt(0, QColor(245, 0, 0));
            rg.setColorAt(0.6, QColor(210, 0, 0));
            rg.setColorAt(1, QColor(166, 0, 0));
        }
        else
        {
            rg.setColorAt(0, QColor(0, 245, 0));
            rg.setColorAt(0.6, QColor(0, 210, 0));
            rg.setColorAt(1, QColor(0, 166, 0));
        }
    }
    painter.setBrush(rg);
    painter.drawEllipse(-radius, -radius, radius << 1, radius << 1);
}

void QcwIndicatorLamp::setAlarm(bool alarm)
{
	m_alarm = alarm;
	update();
}

void QcwIndicatorLamp::setLampState(QcwIndicatorLamp::LampState s)
{
    lamp_state = s;
    update();
}

QcwIndicatorLamp::LampState QcwIndicatorLamp::getLampState()
{
    return lamp_state;
}


