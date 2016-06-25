#include "tcpportsettings.h"
#include "ui_tcpportsettings.h"

#include <QtCore>
#include <QDebug>

TcpPortSettings::TcpPortSettings(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::TcpPortSettings)
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
}

TcpPortSettings::~TcpPortSettings()
{
    delete ui;
}

/*!
 * \brief TcpPortSettings::on_Button_apply_clicked
 */
void TcpPortSettings::on_Button_apply_clicked()
{
    m_hostAddress = ui->lineEdit_ip->text();
    m_port = ui->lineEdit_port->text().toUShort();

    //! Save settings to file: "/SuperConsole.ini"
    QSettings settings(QDir::currentPath() + "/SuperConsole.ini", QSettings::IniFormat);
    qDebug(qPrintable(QDir::currentPath() + "/SuperConsole.ini"));
    settings.beginGroup("TcpPortSettings");
    settings.setValue("HostAddress", m_hostAddress);
    settings.setValue("Port", m_port);
    settings.endGroup();

    emit emitApply();
}
