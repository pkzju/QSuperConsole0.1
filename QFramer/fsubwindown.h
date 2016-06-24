#ifndef FSUBWINDOWN_H
#define FSUBWINDOWN_H

#include <QDialog>
#include <QMouseEvent>
#include <QShowEvent>
#include <QCloseEvent>
#include "ftitlebar.h"
#include <QVBoxLayout>

class FSubWindown : public QDialog
{
    Q_OBJECT
private:
    QPoint dragPosition;

private:
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);

    FTitleBar* titlebar;
    QVBoxLayout* mLayout;

protected:
    void showEvent(QShowEvent * event);
    void closeEvent(QCloseEvent * event);
public:
    QSize normalSize;

public:
    explicit FSubWindown(QWidget *parent = 0);
    void initData();
    void initUI();
    void initConnect();
    FTitleBar* getTitleBar();
    QLayout *layout();

signals:

public slots:
   virtual void animationClose();
};
#endif // FSUBWINDOWN_H
