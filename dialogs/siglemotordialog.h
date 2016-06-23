#ifndef SIGLEMOTORDIALOG_H
#define SIGLEMOTORDIALOG_H

#include "QFramer/fsubwindown.h"
#include "fanmotor/fpublic.h"


class SigleMotorDialog : public FSubWindown
{
    Q_OBJECT
public:
    explicit SigleMotorDialog(QWidget *parent = 0);
    ~SigleMotorDialog();
    void initUI();
    void initConnect();
    static SigleMotorDialog *getS_Instance();
    static void deleteInstance();
signals:

public slots:
    void show(QMotor *motor);

private:
    static SigleMotorDialog *s_Instance;

};


#endif // SIGLEMOTORDIALOG_H
