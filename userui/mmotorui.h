#ifndef MMOTORUI_H
#define MMOTORUI_H

#include <QFrame>

namespace Ui {
class MMotorUi;
}

class MMotorUi : public QFrame
{
    Q_OBJECT

public:
    explicit MMotorUi(QWidget *parent = 0);
    ~MMotorUi();
    static MMotorUi *getS_Instance();

private:
    Ui::MMotorUi *ui;
    static MMotorUi *s_Instance;
};

#endif // MMOTORUI_H
