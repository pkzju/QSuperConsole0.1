#ifndef GROUPMONITORDIALOG_H
#define GROUPMONITORDIALOG_H

#include "QFramer/fsubwindown.h"

class QTableWidget;
class QMotor;
class GroupMonitorUi;

class GroupMonitorDialog : public FSubWindown
{
    Q_OBJECT
public:
    explicit GroupMonitorDialog(QVector<QMotor *> *_motors, QWidget *parent = 0);
    ~GroupMonitorDialog();
    void initUI();
    void initConnect();
    QTableWidget *table();

signals:

public slots:
    void show(int _groupID, QVector<QMotor *> *_motors);

private:
    GroupMonitorUi *mGroupMonitorUi;
    int m_motorNumber;


};
#endif // GROUPMONITORDIALOG_H
