#ifndef TCPPORTSETTINGS_H
#define TCPPORTSETTINGS_H

#include <QFrame>

namespace Ui {
class TcpPortSettings;
}

class TcpPortSettings : public QFrame
{
    Q_OBJECT

public:
    explicit TcpPortSettings(QWidget *parent = 0);
    ~TcpPortSettings();

private slots:
    void on_Button_apply_clicked();
signals:
    void emitApply();

private:
    Ui::TcpPortSettings *ui;

    QString m_hostAddress;
    quint16 m_port;
};

#endif // TCPPORTSETTINGS_H
