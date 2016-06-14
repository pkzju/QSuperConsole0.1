#ifndef HOMEWINDOW_H
#define HOMEWINDOW_H

#include <QWidget>

namespace Ui {
class homewindow;
}

class homewindow : public QWidget
{
    Q_OBJECT

public:
    explicit homewindow(QWidget *parent = 0);
    ~homewindow();

private:
    Ui::homewindow *ui;
};

#endif // HOMEWINDOW_H
