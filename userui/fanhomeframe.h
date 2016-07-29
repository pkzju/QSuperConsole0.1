#ifndef FANHOMEFRAME_H
#define FANHOMEFRAME_H

#include <QFrame>
#include "fanmotor/fpublic.h"

class QButtonGroup;
class QTableWidget;

namespace Ui {
class FanHomeFrame;
}

class FanHomeFrame : public QFrame
{
    Q_OBJECT

public:
    explicit FanHomeFrame(QWidget *parent = 0);
    ~FanHomeFrame();
    void setGroup(FanGroupInfo *first, FanGroupInfo *second);

private slots:
    void enterGroup(int id);

private:
    Ui::FanHomeFrame *ui;
    QButtonGroup *btnGroup;
    QTableWidget *firstTable;
    QTableWidget *secondTable;
    quint16 firstGroupId;
    quint16 secondGroupId;

};

#endif // FANHOMEFRAME_H
