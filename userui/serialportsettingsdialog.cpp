/****************************************************************************
**
**
****************************************************************************/

#include "serialportsettingsdialog.h"
#include "ui_serialportsettingsdialog.h"

#include <QtSerialPort/QSerialPortInfo>
#include <QIntValidator>
#include <QLineEdit>

#include <QtCore>
#include <QDebug>

#include "thread/serialportthread.h"

QT_USE_NAMESPACE

static const char blankString[] = QT_TRANSLATE_NOOP("SerialPortSettingsDialog", "N/A");

SerialPortSettingsDialog *SerialPortSettingsDialog::instance = Q_NULLPTR;

SerialPortSettingsDialog::SerialPortSettingsDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SerialPortSettingsDialog)
{
    ui->setupUi(this);

    intValidator = new QIntValidator(0, 4000000, this);

//    mySerialPortThread = SerialPortThread::getInstance();

    ui->baudRateBox->setInsertPolicy(QComboBox::NoInsert);

    connect(ui->baudRateBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(checkCustomBaudRatePolicy(int)));
    connect(ui->serialPortInfoListBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(checkCustomDevicePathPolicy(int)));

    fillPortsParameters();
    fillPortsInfo();

    QSettings settings(QDir::currentPath() + "/SuperConsole.ini", QSettings::IniFormat);
    settings.beginGroup("SerialPortSettingsDialog");
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
}

SerialPortSettingsDialog::~SerialPortSettingsDialog()
{
    instance = Q_NULLPTR;
    delete ui;
}

SerialPortSettingsDialog* SerialPortSettingsDialog::getInstance()
{
    if(!instance)
    {
        instance = new SerialPortSettingsDialog();
    }
    return instance;
}

SerialPortSettings::Settings SerialPortSettingsDialog::settings() const
{
    return currentSettings;
}

void SerialPortSettingsDialog::setSettings(SerialPortSettings::Settings s)
{
    currentSettings = s;
    ui->serialPortInfoListBox->setCurrentText(s.name);
    ui->baudRateBox->setCurrentText(s.stringBaudRate);
    ui->dataBitsBox->setCurrentText(s.stringDataBits);
    ui->parityBox->setCurrentText(s.stringParity);
    ui->stopBitsBox->setCurrentText(s.stringStopBits);
    ui->flowControlBox->setCurrentText(s.stringFlowControl);

}
void SerialPortSettingsDialog::checkCustomBaudRatePolicy(int idx)
{
    bool isCustomBaudRate = !ui->baudRateBox->itemData(idx).isValid();
    ui->baudRateBox->setEditable(isCustomBaudRate);
    if (isCustomBaudRate) {
        ui->baudRateBox->clearEditText();
        QLineEdit *edit = ui->baudRateBox->lineEdit();
        edit->setValidator(intValidator);
    }
}

void SerialPortSettingsDialog::checkCustomDevicePathPolicy(int idx)
{
    bool isCustomPath = !ui->serialPortInfoListBox->itemData(idx).isValid();
    ui->serialPortInfoListBox->setEditable(isCustomPath);
    if (isCustomPath)
        ui->serialPortInfoListBox->clearEditText();
}

void SerialPortSettingsDialog::fillPortsParameters()
{
    ui->baudRateBox->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
    ui->baudRateBox->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
    ui->baudRateBox->addItem(QStringLiteral("38400"), QSerialPort::Baud38400);
    ui->baudRateBox->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);
    ui->baudRateBox->addItem(tr("Custom"));
//    ui->baudRateBox->setCurrentIndex(3);

    ui->dataBitsBox->addItem(QStringLiteral("5"), QSerialPort::Data5);
    ui->dataBitsBox->addItem(QStringLiteral("6"), QSerialPort::Data6);
    ui->dataBitsBox->addItem(QStringLiteral("7"), QSerialPort::Data7);
    ui->dataBitsBox->addItem(QStringLiteral("8"), QSerialPort::Data8);
//    ui->dataBitsBox->setCurrentIndex(3);

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

void SerialPortSettingsDialog::fillPortsInfo()
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

void SerialPortSettingsDialog::updateSettings()
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

void SerialPortSettingsDialog::on_searchButton_clicked()
{
    fillPortsInfo();
    ui->serialPortInfoListBox->showPopup();
}

void SerialPortSettingsDialog::apply()
{
    updateSettings();

    QSettings settings(QDir::currentPath() + "/SuperConsole.ini", QSettings::IniFormat);
    qDebug(qPrintable(QDir::currentPath() + "/SuperConsole.ini"));
    settings.beginGroup("SerialPortSettingsDialog");
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


   // hide();
    emit emitApply();
}

void SerialPortSettingsDialog::on_Button_apply_clicked()
{
    apply();
}
