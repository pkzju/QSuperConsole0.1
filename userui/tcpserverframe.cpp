﻿#include "tcpserverframe.h"
#include "ui_tcpserverframe.h"

TcpServerFrame::TcpServerFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::TcpServerFrame)
  ,m_tcpServer{new QTcpServer(this)}
  ,m_currentTcpServerSocket{Q_NULLPTR}
{
    ui->setupUi(this);

    connect(m_tcpServer, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
}

TcpServerFrame::~TcpServerFrame()
{
    delete ui;
}

/*!
 * \brief TcpServerFrame::on_pushButton_start_clicked
 */
void TcpServerFrame::on_pushButton_start_clicked()
{
    quint16 _port = 6474;
    if(!m_tcpServer->listen(QHostAddress::Any, _port)){
        return;
    }
    ui->pushButton_start->setEnabled(false);
    ui->pushButton_close->setEnabled(true);

}

/*!
 * \brief TcpServerFrame::on_pushButton_close_clicked
 */
void TcpServerFrame::on_pushButton_close_clicked()
{
    if(m_tcpServer->isListening()){
        m_tcpServer->close();
        ui->pushButton_close->setEnabled(false);
        ui->pushButton_start->setEnabled(true);
    }
}

/*!
 * \brief TcpServerFrame::onNewConnection
 */
void TcpServerFrame::onNewConnection()
{
    QTcpSocket * _tcpServerSocket = m_tcpServer->nextPendingConnection();
    m_currentTcpServerSocket = _tcpServerSocket;
    connect(_tcpServerSocket, SIGNAL(disconnected()),  _tcpServerSocket, SLOT(deleteLater()));
    connect(_tcpServerSocket, SIGNAL(readyRead()), this, SLOT(readSocked()));
}

/*!
 * \brief TcpServerFrame::readSocked
 */
void TcpServerFrame::readSocked()
{
    QTcpSocket *_tcpSocked = qobject_cast<QTcpSocket*>(sender());

    //! Constructs a data stream that has m_tcpClientSocked I/O device
    QDataStream in(_tcpSocked);

    //! Sets the version number of the data serialization format
    in.setVersion(QDataStream::Qt_5_7);

    //! Starts a new read transaction on the stream
    in.startTransaction();

    quint16 length;
    QString str;
    in >> length >> str;

    if (!in.commitTransaction())
        return;     //!< wait for more data

    ui->textBrowser->append(str);
}

/*!
 * \brief TcpServerFrame::on_pushButton_send_clicked
 */
void TcpServerFrame::on_pushButton_send_clicked()
{
    if(!m_currentTcpServerSocket || m_currentTcpServerSocket->state() != QAbstractSocket::ConnectedState)
        return;

    QByteArray block;
    QDataStream out(&block,QIODevice::ReadWrite);
    out.setVersion(QDataStream::Qt_5_7);

    //! Setting the initial value of the transmission length is 0
    out << (quint16) 0 << ui->textEdit->toPlainText();//!< Value of the transmission

    //! Returns to the starting position of the byte stream
    out.device()->seek(0);

    //! Reset byte stream length
    out << (quint16) (block.size()-sizeof(quint16));

    //! Write data to the socket cache and send
    m_currentTcpServerSocket->write(block);

}