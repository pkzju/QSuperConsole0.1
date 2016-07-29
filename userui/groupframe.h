#ifndef GROUPFRAME_H
#define GROUPFRAME_H

#include <QFrame>

class QTableWidget;
class QPushButton;
class QLCDNumber;

namespace Ui {
class GroupFrame;
}

class GroupFrame : public QFrame
{
    Q_OBJECT

public:
    explicit GroupFrame(QWidget *parent = 0);
    ~GroupFrame();

    void setTemperature(float t);
    void setHumidity(float h);
    QPushButton *getGroupBtn();

    QTableWidget *getTable();
private:
    Ui::GroupFrame *ui;
};

#endif // GROUPFRAME_H
