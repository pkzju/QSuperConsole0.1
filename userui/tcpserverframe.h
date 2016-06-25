#ifndef TCPSERVERFRAME_H
#define TCPSERVERFRAME_H

#include <QFrame>

#include <QTcpSocket>
#include <QTcpServer>

namespace Ui {
class TcpServerFrame;
}

class TcpServerFrame : public QFrame
{
    Q_OBJECT

public:
    explicit TcpServerFrame(QWidget *parent = 0);
    ~TcpServerFrame();

private slots:
    void on_pushButton_start_clicked();

    void on_pushButton_close_clicked();

    void onNewConnection();

    void readSocked();
    void on_pushButton_send_clicked();

private:
    Ui::TcpServerFrame *ui;
    QTcpServer *m_tcpServer;
    QTcpSocket *m_currentTcpServerSocket;
};

#endif // TCPSERVERFRAME_H
