#ifndef TCPCLIENTFRAME_H
#define TCPCLIENTFRAME_H

#include <QFrame>

#include <QTcpSocket>

namespace Ui {
class TcpClientFrame;
}

class TcpClientFrame : public QFrame
{
    Q_OBJECT

public:
    explicit TcpClientFrame(QWidget *parent = 0);
    ~TcpClientFrame();

private slots:
    void on_pushButton_connect_clicked();
    void on_pushButton_disconnect_clicked();
    void readSocked();
    void onTcpSockedStateChange(int state);

    void on_pushButton_send_clicked();

    void dispalyError(QAbstractSocket::SocketError socketError);


private:
    Ui::TcpClientFrame *ui;

    QTcpSocket *m_tcpClientSocked;
};

#endif // TCPCLIENTFRAME_H
