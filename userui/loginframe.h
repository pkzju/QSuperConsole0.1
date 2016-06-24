#ifndef LOGINFRAME_H
#define LOGINFRAME_H

#include <QFrame>

namespace Ui {
class LoginFrame;
}

class LoginFrame : public QFrame
{
    Q_OBJECT

public:
    explicit LoginFrame(QWidget *parent = 0);
    ~LoginFrame();

signals:
    void login();

private slots:
    void on_pushButton_clicked();


private:
    Ui::LoginFrame *ui;
};

#endif // LOGOFRAME_H
