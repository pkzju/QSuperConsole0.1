#include "siglemotordialog.h"


#include <QVBoxLayout>
#include "userui/siglemotorframe.h"


SigleMotorDialog *SigleMotorDialog::s_Instance = nullptr;

SigleMotorDialog::SigleMotorDialog(QWidget *parent):
    FSubWindown(parent)
{
    initUI();

}

SigleMotorDialog::~SigleMotorDialog()
{
    SigleMotorFrame::deleteInstance();
    s_Instance = Q_NULLPTR;
}

void SigleMotorDialog::initUI()
{
    normalSize = QSize(1080, 640);
    setSizeGripEnabled(true);
    getTitleBar()->getTitleLabel()->setText(tr("Fan"));
    QVBoxLayout* mainLayout = (QVBoxLayout*)layout();

    SigleMotorFrame *motorui = SigleMotorFrame::getInstance();
    mainLayout->addWidget(motorui);
}

SigleMotorDialog *SigleMotorDialog::getS_Instance()
{
    if(!s_Instance)
    {
        s_Instance = new SigleMotorDialog;
    }
    return s_Instance;
}

void SigleMotorDialog::deleteInstance()
{
    if(s_Instance)
        s_Instance->deleteLater();
}

void SigleMotorDialog::show(QMotor *motor)
{
    getTitleBar()->getTitleLabel()->setText(tr("Fan%1").arg(motor->m_address));

    if(FSubWindown::isHidden())
        FSubWindown::show();
    else{
        FSubWindown::hide();
        FSubWindown::show();
    }
    SigleMotorFrame::getInstance()->changeMotor(motor);

}





