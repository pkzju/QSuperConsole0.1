#include "siglegroupdialog.h"

#include <QVBoxLayout>
#include "userui/fanmotorui.h"

sigleGroupDialog *sigleGroupDialog::s_Instance = Q_NULLPTR;

sigleGroupDialog::sigleGroupDialog(QWidget *parent):
    FSubWindown(parent)
{
    qDebug("sigleGroupDialog init");
    initUI();

}

sigleGroupDialog::~sigleGroupDialog()
{
    qDebug("sigleGroupDialog exit");
    FanMotorUi::deleteInstance();
    s_Instance = Q_NULLPTR;
}

void sigleGroupDialog::initUI()
{
    normalSize = QSize(850, 730);
    setSizeGripEnabled(true);
    getTitleBar()->getTitleLabel()->setText(tr("Group"));
    QVBoxLayout* mainLayout = (QVBoxLayout*)layout();

    FanMotorUi *fanmotorui = FanMotorUi::getS_Instance();
    mainLayout->addWidget(fanmotorui);
}

sigleGroupDialog *sigleGroupDialog::getS_Instance()
{
    if(!s_Instance)
    {
        s_Instance = new sigleGroupDialog;
    }
    return s_Instance;
}

void sigleGroupDialog::deleteInstance()
{
    if(s_Instance)
        s_Instance->deleteLater();
}

void sigleGroupDialog::show(FanGroupInfo *group)
{
    getTitleBar()->getTitleLabel()->setText(tr("Group%1").arg(group->m_groupID));
    FanMotorUi::getS_Instance()->changeGroup(group);
    if(FSubWindown::isHidden())
        FSubWindown::show();
    else{
        FSubWindown::hide();
        FSubWindown::show();
    }

}
