#include "fsetpushbutton.h"


FSetPushButton::FSetPushButton(const QString& text)
{
    setFlat(true);
    setCheckable(true);
    setFixedSize(40, 40);
 //   setText(text);
    setFocusPolicy(Qt::NoFocus);
    setObjectName("checkable");
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

