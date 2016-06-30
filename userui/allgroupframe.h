#ifndef ALLGROUPFRAME_H
#define ALLGROUPFRAME_H

#include <QFrame>

namespace Ui {
class AllGroupFrame;
}

class AllGroupFrame : public QFrame
{
    Q_OBJECT

public:
    explicit AllGroupFrame(QWidget *parent = 0);
    ~AllGroupFrame();

private:
    Ui::AllGroupFrame *ui;
};

#endif // ALLGROUPFRAME_H
