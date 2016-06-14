#ifndef QCANOPEN_H
#define QCANOPEN_H

#include <QWidget>

namespace Ui {
class QCANopen;
}

class QCANopen : public QWidget
{
    Q_OBJECT

public:
    explicit QCANopen(QWidget *parent = 0);
    ~QCANopen();

private:
    Ui::QCANopen *ui;
};

#endif // QCANOPEN_H
