#include "siglegroupdialog.h"

#include <QVBoxLayout>
#include "userui/fanmotorui.h"


sigleGroupDialog *sigleGroupDialog::s_Instance = nullptr;

sigleGroupDialog::sigleGroupDialog(QWidget *parent):
    FSubWindown(parent)
{
    initUI();

}

sigleGroupDialog::~sigleGroupDialog()
{

}

void sigleGroupDialog::initUI()
{
    normalSize = QSize(1200, 800);
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
