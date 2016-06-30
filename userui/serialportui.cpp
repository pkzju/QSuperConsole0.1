#include "serialportui.h"
#include "ui_serialportui.h"

#include <QtSerialPort/QSerialPortInfo>
#include <QIntValidator>
#include <QLineEdit>
#include <QtCore>

/*!
 * \brief blankString
 */
static const char blankString[] = QT_TRANSLATE_NOOP("SerialPortUi", "N/A");

/*!
 * \brief SerialPortUi::SerialPortUi
 * \param parent
 */
SerialPortUi::SerialPortUi(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SerialPortUi)
{
    qDebug("SerialPortUi init");
    ui->setupUi(this);
    initData();
    initUI();
    initThread();
    initConnect();
}

/*!
 * \brief SerialPortUi::initData
 */
void SerialPortUi::initData()
{
    mySerialPortThread = SerialPortThread::getInstance();

}

/*!
 * \brief SerialPortUi::initUI
 */
void SerialPortUi::initUI()
{
    intValidator = new QIntValidator(0, 4000000, this);

    ui->baudRateBox->setInsertPolicy(QComboBox::NoInsert);

    connect(ui->serialPortInfoListBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(showPortInfo(int)));
    connect(ui->baudRateBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(checkCustomBaudRatePolicy(int)));
    connect(ui->serialPortInfoListBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(checkCustomDevicePathPolicy(int)));

    fillPortsParameters();
    fillPortsInfo();

    QSettings settings(QDir::currentPath() + "/SuperConsole.ini", QSettings::IniFormat);
    settings.beginGroup("SerialPortUi");
    currentSettings.name = settings.value("SerialPortname", "").toString();
    currentSettings.baudRate = static_cast<QSerialPort::BaudRate>(settings.value("SerialPortBaudRate", QSerialPort::Baud9600).toInt());
    currentSettings.dataBits = static_cast<QSerialPort::DataBits>(settings.value("SerialPortDataBits", QSerialPort::Data8).toInt());
    currentSettings.parity = static_cast<QSerialPort::Parity>(settings.value("SerialPortParity", QSerialPort::NoParity).toInt());
    currentSettings.stopBits = static_cast<QSerialPort::StopBits>(settings.value("SerialPortStopBits", QSerialPort::OneStop).toInt());
    currentSettings.flowControl = static_cast<QSerialPort::FlowControl>(settings.value("SerialPortFlowControl", QSerialPort::NoFlowControl).toInt());

    currentSettings.stringBaudRate = settings.value("SerialPortBaudRateString", "9600").toString();
    currentSettings.stringDataBits = settings.value("SerialPortDataBitsString", "8").toString();
    currentSettings.stringParity = settings.value("SerialPortParityString", "None").toString();
    currentSettings.stringStopBits = settings.value("SerialPortStopBitsString", "1").toString();
    currentSettings.stringFlowControl = settings.value("SerialPortFlowControlString", "None").toString();
    settings.endGroup();

    ui->serialPortInfoListBox->setCurrentText(currentSettings.name);
    ui->baudRateBox->setCurrentText(currentSettings.stringBaudRate);
    ui->dataBitsBox->setCurrentText(currentSettings.stringDataBits);
    ui->parityBox->setCurrentText(currentSettings.stringParity);
    ui->stopBitsBox->setCurrentText(currentSettings.stringStopBits);
    ui->flowControlBox->setCurrentText(currentSettings.stringFlowControl);

    updateSettings();

    updateSettings();

    ui->Button_Close->setEnabled(false);
    ui->pushButton_Send->setEnabled(false);

    ui->textBrowser->document ()->setMaximumBlockCount (50);
}

/*!
 * \brief SerialPortUi::initThread
 */
void SerialPortUi::initThread()
{

}

/*!
 * \brief SerialPortUi::initConnect
 */
void SerialPortUi::initConnect()
{
    connect(mySerialPortThread, SIGNAL(message(QString)),this, SLOT(messageShow(QString)));
    connect(mySerialPortThread, SIGNAL(message(QByteArray)),this, SLOT(messageShow(QByteArray)));
}

/*!
 * \brief SerialPortUi::messageShow
 * \param s
 */
void SerialPortUi::messageShow(const QString &s)
{

    ui->textBrowser->append(s);
    if(s == "open successfully !"){
        ui->Button_Open->setEnabled(false);
        ui->Button_Close->setEnabled(true);
        ui->pushButton_Send->setEnabled(true);
    }
    if(s == "close successfully !"){
        ui->Button_Open->setEnabled(true);
        ui->Button_Close->setEnabled(false);
        ui->pushButton_Send->setEnabled(false);
    }
}

void SerialPortUi::messageShow(const QByteArray &data)
{
    QString s;
    if(ui->radioButton_HEXREC->isChecked()){
        s.append("0x");
        for(int i = 0; i < data.size(); i++){
            s.append(tr("%1 ").arg(QString::number(data.at(i), 16)));
        }
    }
    else{
        s.append(data);
    }

    ui->textBrowser->append(s);

}

/*!
 * \brief SerialPortUi::~SerialPortUi
 */
SerialPortUi::~SerialPortUi()
{
    qDebug("SerialPortUi exit");
    if(mySerialPortThread)
        mySerialPortThread->deleteLater();

    QSettings settings(QDir::currentPath() + "/SuperConsole.ini", QSettings::IniFormat);
    qDebug(qPrintable(QDir::currentPath() + "/SuperConsole.ini"));
    settings.beginGroup("SerialPortUi");
    settings.setValue("SerialPortname", this->settings().name);
    settings.setValue("SerialPortBaudRate", this->settings().baudRate);
    settings.setValue("SerialPortDataBits", this->settings().dataBits);
    settings.setValue("SerialPortParity", this->settings().parity);
    settings.setValue("SerialPortStopBits", this->settings().stopBits);
    settings.setValue("SerialPortFlowControl", this->settings().flowControl);

    settings.setValue("SerialPortBaudRateString", this->settings().stringBaudRate);
    settings.setValue("SerialPortDataBitsString", this->settings().stringDataBits);
    settings.setValue("SerialPortParityString", this->settings().stringParity);
    settings.setValue("SerialPortStopBitsString", this->settings().stringStopBits);
    settings.setValue("SerialPortFlowControlString", this->settings().stringFlowControl);
    settings.endGroup();

    delete ui;
}

/*!
 * \brief SerialPortUi::on_searchButton_clicked
 */
void SerialPortUi::on_searchButton_clicked()
{
    fillPortsInfo();
    ui->serialPortInfoListBox->showPopup();
}

/*!
 * \brief SerialPortUi::on_Button_Close_clicked
 */
void SerialPortUi::on_Button_Close_clicked()
{
    mySerialPortThread = SerialPortThread::getInstance();
    mySerialPortThread->mStop();
}

/*!
 * \brief SerialPortUi::on_Button_Open_clicked
 */
void SerialPortUi::on_Button_Open_clicked()
{
    updateSettings();
    mySerialPortThread = SerialPortThread::getInstance();
    mySerialPortThread->mStart(currentSettings);

}

/*!
 * \brief SerialPortUi::settings
 * \return
 */
SerialPortSettings::Settings SerialPortUi::settings() const
{
    return currentSettings;
}

/*!
 * \brief SerialPortUi::setSettings
 * \param s
 */
void SerialPortUi::setSettings(SerialPortSettings::Settings s)
{
    currentSettings = s;
    ui->serialPortInfoListBox->setCurrentText(s.name);
    ui->baudRateBox->setCurrentText(s.stringBaudRate);
    ui->dataBitsBox->setCurrentText(s.stringDataBits);
    ui->parityBox->setCurrentText(s.stringParity);
    ui->stopBitsBox->setCurrentText(s.stringStopBits);
    ui->flowControlBox->setCurrentText(s.stringFlowControl);

}

/*!
 * \brief SerialPortUi::fillPortsParameters
 */
void SerialPortUi::fillPortsParameters()
{
    ui->baudRateBox->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
    ui->baudRateBox->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
    ui->baudRateBox->addItem(QStringLiteral("38400"), QSerialPort::Baud38400);
    ui->baudRateBox->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);
    ui->baudRateBox->addItem(tr("Custom"));
    ui->baudRateBox->setCurrentIndex(3);

    ui->dataBitsBox->addItem(QStringLiteral("5"), QSerialPort::Data5);
    ui->dataBitsBox->addItem(QStringLiteral("6"), QSerialPort::Data6);
    ui->dataBitsBox->addItem(QStringLiteral("7"), QSerialPort::Data7);
    ui->dataBitsBox->addItem(QStringLiteral("8"), QSerialPort::Data8);
    ui->dataBitsBox->setCurrentIndex(3);

    ui->parityBox->addItem(tr("None"), QSerialPort::NoParity);
    ui->parityBox->addItem(tr("Even"), QSerialPort::EvenParity);
    ui->parityBox->addItem(tr("Odd"), QSerialPort::OddParity);
    ui->parityBox->addItem(tr("Mark"), QSerialPort::MarkParity);
    ui->parityBox->addItem(tr("Space"), QSerialPort::SpaceParity);

    ui->stopBitsBox->addItem(QStringLiteral("1"), QSerialPort::OneStop);
#ifdef Q_OS_WIN
    ui->stopBitsBox->addItem(tr("1.5"), QSerialPort::OneAndHalfStop);
#endif
    ui->stopBitsBox->addItem(QStringLiteral("2"), QSerialPort::TwoStop);

    ui->flowControlBox->addItem(tr("None"), QSerialPort::NoFlowControl);
    ui->flowControlBox->addItem(tr("RTS/CTS"), QSerialPort::HardwareControl);
    ui->flowControlBox->addItem(tr("XON/XOFF"), QSerialPort::SoftwareControl);
}

/*!
 * \brief SerialPortUi::fillPortsInfo
 */
void SerialPortUi::fillPortsInfo()
{
    ui->serialPortInfoListBox->clear();
    QString description;
    QString manufacturer;
    QString serialNumber;
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        QStringList list;
        description = info.description();
        manufacturer = info.manufacturer();
        serialNumber = info.serialNumber();
        list << info.portName()
             << (!description.isEmpty() ? description : blankString)
             << (!manufacturer.isEmpty() ? manufacturer : blankString)
             << (!serialNumber.isEmpty() ? serialNumber : blankString)
             << info.systemLocation()
             << (info.vendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : blankString)
             << (info.productIdentifier() ? QString::number(info.productIdentifier(), 16) : blankString);

        ui->serialPortInfoListBox->addItem(list.first(), list);
    }

    ui->serialPortInfoListBox->addItem(tr("Custom"));
}

/*!
 * \brief SerialPortUi::updateSettings
 */
void SerialPortUi::updateSettings()
{
    currentSettings.name = ui->serialPortInfoListBox->currentText();

    if (ui->baudRateBox->currentIndex() == 4) {
        currentSettings.baudRate = ui->baudRateBox->currentText().toInt();
    } else {
        currentSettings.baudRate = static_cast<QSerialPort::BaudRate>(
                    ui->baudRateBox->itemData(ui->baudRateBox->currentIndex()).toInt());
    }
    currentSettings.stringBaudRate = QString::number(currentSettings.baudRate);

    currentSettings.dataBits = static_cast<QSerialPort::DataBits>(
                ui->dataBitsBox->itemData(ui->dataBitsBox->currentIndex()).toInt());
    currentSettings.stringDataBits = ui->dataBitsBox->currentText();

    currentSettings.parity = static_cast<QSerialPort::Parity>(
                ui->parityBox->itemData(ui->parityBox->currentIndex()).toInt());
    currentSettings.stringParity = ui->parityBox->currentText();

    currentSettings.stopBits = static_cast<QSerialPort::StopBits>(
                ui->stopBitsBox->itemData(ui->stopBitsBox->currentIndex()).toInt());
    currentSettings.stringStopBits = ui->stopBitsBox->currentText();

    currentSettings.flowControl = static_cast<QSerialPort::FlowControl>(
                ui->flowControlBox->itemData(ui->flowControlBox->currentIndex()).toInt());
    currentSettings.stringFlowControl = ui->flowControlBox->currentText();

}

/*!
 * \brief SerialPortUi::showPortInfo
 * \param idx
 */
void SerialPortUi::showPortInfo(int idx)
{
    if (idx == -1)
        return;

    QStringList list = ui->serialPortInfoListBox->itemData(idx).toStringList();
    ui->descriptionLabel->setText(tr("Description: %1").arg(list.count() > 1 ? list.at(1) : tr(blankString)));
    ui->manufacturerLabel->setText(tr("Manufacturer: %1").arg(list.count() > 2 ? list.at(2) : tr(blankString)));
    ui->serialNumberLabel->setText(tr("Serial number: %1").arg(list.count() > 3 ? list.at(3) : tr(blankString)));
    ui->locationLabel->setText(tr("Location: %1").arg(list.count() > 4 ? list.at(4) : tr(blankString)));
    ui->vidLabel->setText(tr("Vendor Identifier: %1").arg(list.count() > 5 ? list.at(5) : tr(blankString)));
    ui->pidLabel->setText(tr("Product Identifier: %1").arg(list.count() > 6 ? list.at(6) : tr(blankString)));
}

/*!
 * \brief SerialPortUi::checkCustomBaudRatePolicy
 * \param idx
 */
void SerialPortUi::checkCustomBaudRatePolicy(int idx)
{
    bool isCustomBaudRate = !ui->baudRateBox->itemData(idx).isValid();
    ui->baudRateBox->setEditable(isCustomBaudRate);
    if (isCustomBaudRate) {
        ui->baudRateBox->clearEditText();
        QLineEdit *edit = ui->baudRateBox->lineEdit();
        edit->setValidator(intValidator);
    }
}

/*!
 * \brief SerialPortUi::checkCustomDevicePathPolicy
 * \param idx
 */
void SerialPortUi::checkCustomDevicePathPolicy(int idx)
{
    bool isCustomPath = !ui->serialPortInfoListBox->itemData(idx).isValid();
    ui->serialPortInfoListBox->setEditable(isCustomPath);
    if (isCustomPath)
        ui->serialPortInfoListBox->clearEditText();
}

/*!
 * \brief SerialPortUi::on_pushButton_Clear_clicked
 */
void SerialPortUi::on_pushButton_Clear_clicked()
{
    ui->textEdit_Send->clear();
}

/*!
 * \brief SerialPortUi::on_pushButton_ClearRec_clicked
 */
void SerialPortUi::on_pushButton_ClearRec_clicked()
{
    ui->textBrowser->clear();
}

void SerialPortUi::on_pushButton_Send_clicked()
{
    if(!mySerialPortThread || !mySerialPortThread->isRunning())
        return;

    QByteArray _data;
    if(ui->radioButton_HEX->isChecked()){
        QString str;
        bool ok;
        int _hex;
        QStringList _hexlist;
        str = ui->textEdit_Send->toPlainText();

        //! Splits the string into substrings wherever " " occurs
        _hexlist = str.split(" ", QString::SkipEmptyParts);
        QStringList::const_iterator constIterator;
        for (constIterator = _hexlist.constBegin(); constIterator != _hexlist.constEnd();
             ++constIterator){
            _hex = (*constIterator).toInt(&ok, 16);

            if(!ok || _hex > 255 || _hex < 0){
                ui->textBrowser->append(tr("Input data have wrong format !"));
                return;
            }
            _data.append(_hex);
            qDebug()<< *constIterator << _hex;
        }
    }else{
        _data = ui->textEdit_Send->toPlainText().toUtf8();

    }


    mySerialPortThread->mSendData(_data);
    messageShow(_data);

}
