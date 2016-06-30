
#include "tcpclientframe.h"
#include "ui_tcpclientframe.h"

#include <QtCore>

#include <QHostAddress>

TcpClientFrame::TcpClientFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::TcpClientFrame)
  ,m_tcpClientSocked(new QTcpSocket(this))
{
    ui->setupUi(this);

    //! Read settings from file: "/SuperConsole.ini"
    QSettings settings(QDir::currentPath() + "/SuperConsole.ini", QSettings::IniFormat);
    settings.beginGroup("TcpPortSettings");
    m_hostAddress = settings.value("HostAddress", "127.0.0.1").toString();
    m_port = settings.value("Port", "6474").toUInt();
    settings.endGroup();

    ui->lineEdit_ip->setText(m_hostAddress);
    ui->lineEdit_port->setText(QString::number(m_port));

    onTcpSockedStateChange(QAbstractSocket::UnconnectedState);

    connect(m_tcpClientSocked, SIGNAL(readyRead()), this, SLOT(readSocked()));
    connect(m_tcpClientSocked, SIGNAL(stateChanged(QAbstractSocket::SocketState)), \
            this, SLOT(onTcpSockedStateChange(QAbstractSocket::SocketState)));
    connect(m_tcpClientSocked, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(dispalyError(QAbstractSocket::SocketError)));

    ui->textBrowser->document ()->setMaximumBlockCount (50);

}

TcpClientFrame::~TcpClientFrame()
{
    //! Save settings to file: "/SuperConsole.ini"
    QSettings settings(QDir::currentPath() + "/SuperConsole.ini", QSettings::IniFormat);
    qDebug(qPrintable(QDir::currentPath() + "/SuperConsole.ini"));
    settings.beginGroup("TcpPortSettings");
    settings.setValue("HostAddress", m_hostAddress);
    settings.setValue("Port", m_port);
    settings.endGroup();

    delete ui;
}

/*!
 * \brief TcpClientFrame::on_pushButton_connect_clicked
 */
void TcpClientFrame::on_pushButton_connect_clicked()
{
    m_tcpClientSocked->abort();//!< Aborts the current connection and resets the socket

    m_hostAddress = ui->lineEdit_ip->text();
    m_port = ui->lineEdit_port->text().toUShort();

    QHostAddress _hostAddress{m_hostAddress};

    //! Attempts to make a connection to hostName on the given port
    m_tcpClientSocked->connectToHost(_hostAddress, m_port);

}

/*!
 * \brief TcpClientFrame::on_pushButton_disconnect_clicked
 */
void TcpClientFrame::on_pushButton_disconnect_clicked()
{
    m_tcpClientSocked->disconnectFromHost();//!< Attempts to close the socket
}

/*!
 * \brief TcpClientFrame::readSocked
 */
void TcpClientFrame::readSocked()
{
    //! Constructs a data stream that has m_tcpClientSocked I/O device
    QDataStream in(m_tcpClientSocked);

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
 * \brief TcpClientFrame::onTcpSockedStateChange
 * \param state 0:unconnected
 */
void TcpClientFrame::onTcpSockedStateChange(QAbstractSocket::SocketState state)
{
    bool connected = (state != QAbstractSocket::UnconnectedState);
    ui->pushButton_connect->setEnabled(!connected);
    ui->pushButton_disconnect->setEnabled(connected);
}

/*!
 * \brief TcpClientFrame::on_pushButton_send_clicked
 */
void TcpClientFrame::on_pushButton_send_clicked()
{
    if(m_tcpClientSocked->state() != QAbstractSocket::ConnectedState)
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
    m_tcpClientSocked->write(block);

}

/*!
 * \brief TcpClientFrame::dispalyError
 * \param socketError
 */
void TcpClientFrame::dispalyError(QAbstractSocket::SocketError socketError)
{
    switch (socketError)
    {
    case QAbstractSocket::RemoteHostClosedError:
        ui->textBrowser->append(tr("RemoteHostClosedError"));
        break;
    case QAbstractSocket::HostNotFoundError:
        ui->textBrowser->append(tr("The host was not found. Please check the "
                                    "host name and port settings."));
        break;
    case QAbstractSocket::ConnectionRefusedError:
        ui->textBrowser->append(tr("The connection was refused by the peer. "
                                    "Make sure the fortune server is running, "
                                    "and check that the host name and port "
                                    "settings are correct."));
        break;
    default:
        ui->textBrowser->append(tr("The following error occurred: %1.")
                                 .arg(m_tcpClientSocked->errorString()));
    }
}

/*!
 * \brief TcpClientFrame::on_pushButton_clear_clicked
 */
void TcpClientFrame::on_pushButton_clear_clicked()
{
    ui->textBrowser->clear();
}
