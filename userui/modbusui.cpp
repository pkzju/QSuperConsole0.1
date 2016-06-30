#include "modbusui.h"
#include "ui_modbusui.h"
#include "userui/writeregistermodel.h"
#include <QModbusTcpClient>
#include <QModbusRtuSerialMaster>
#include <QStandardItemModel>
#include <QUrl>

#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QIntValidator>


#include <QDebug>

enum ModbusConnection {
    Serial,
    Tcp
};

ModbusUi* ModbusUi::instance = Q_NULLPTR;

ModbusUi::ModbusUi(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ModbusUi),
    lastRequest(Q_NULLPTR),
    modbusDevice(Q_NULLPTR),
    writeModel(new WriteRegisterModel(this)),
    intValidator(new QIntValidator(0, 4000000, this))
{
    ui->setupUi(this);

    //~~Init serialport settings~~
    ui->baudCombo->setInsertPolicy(QComboBox::NoInsert);
    fillPortsParameters();
    fillPortsInfo();
    connect(ui->baudCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(checkCustomBaudRatePolicy(int)));
    connect(ui->serialPortInfoListBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(checkCustomDevicePathPolicy(int)));

    //~~Init write table~~
    writeModel->setStartAddress(ui->writeAddress->value());
    writeModel->setNumberOfValues(ui->writeSize->currentText());
    ROW_COUNT = writeModel->rowCount();

    ui->writeValueTable->setModel(writeModel);
    ui->writeValueTable->hideColumn(1);
//    writeModel->setData(writeModel->index(2,2), QString::number(0xff), Qt::EditRole);

    connect(writeModel, &WriteRegisterModel::updateViewport, ui->writeValueTable->viewport(),
        static_cast<void (QWidget::*)()>(&QWidget::update));

    //~~Init table of write function~~
    ui->writeTable->addItem(tr("Coils"), QModbusDataUnit::Coils);
    ui->writeTable->addItem(tr("Discrete Inputs"), QModbusDataUnit::DiscreteInputs);
    ui->writeTable->addItem(tr("Input Registers"), QModbusDataUnit::InputRegisters);
    ui->writeTable->addItem(tr("Holding Registers"), QModbusDataUnit::HoldingRegisters);
    ui->writeTable->setCurrentIndex(3);

    ui->connectType->setCurrentIndex(0);
    on_connectType_currentIndexChanged(0);

    //~~Init comboBox of writeSize~~
    QStandardItemModel *model = new QStandardItemModel(ROW_COUNT, 1, this);
    for (int i = 0; i < ROW_COUNT; ++i)
        model->setItem(i, new QStandardItem(QStringLiteral("%1").arg(i + 1)));
    ui->writeSize->setModel(model);
    connect(ui->writeSize,&QComboBox::currentTextChanged, writeModel,
        &WriteRegisterModel::setNumberOfValues);
    ui->writeSize->setCurrentText(tr("%1").arg(3));

    //~~Init comboBox of readSize~~
    QStandardItemModel *model2 = new QStandardItemModel(ROW_COUNT, 1, this);
    for (int i = 0; i < ROW_COUNT; ++i)
        model2->setItem(i, new QStandardItem(QStringLiteral("%1").arg(i + 1)));
    ui->readSize->setModel(model2);
    ui->readSize->setCurrentText(tr("%1").arg(3));

    auto valueChanged = static_cast<void (QSpinBox::*)(int)> (&QSpinBox::valueChanged);
    connect(ui->writeAddress, valueChanged, writeModel, &WriteRegisterModel::setStartAddress);

}

void ModbusUi::on_connectType_currentIndexChanged(int index)
{
    if (modbusDevice) {
        modbusDevice->disconnectDevice();
        delete modbusDevice;
        modbusDevice = Q_NULLPTR;
    }

    ModbusConnection type = static_cast<ModbusConnection> (index);
    if (type == Serial) {
        modbusDevice = new QModbusRtuSerialMaster(this);
    } else if (type == Tcp) {
        modbusDevice = new QModbusTcpClient(this);
        if (ui->portEdit->text().isEmpty())
            ui->portEdit->setText(QLatin1Literal("127.0.0.1:502"));
    }

//    connect(modbusDevice, &QModbusClient::errorOccurred, [this](QModbusDevice::Error) {
//        statusBar()->showMessage(modbusDevice->errorString(), 5000);
//    });

    if (!modbusDevice) {
        ui->connectButton->setDisabled(true);
//        if (type == Serial)
//            statusBar()->showMessage(tr("Could not create Modbus master."), 5000);
//        else
//            statusBar()->showMessage(tr("Could not create Modbus client."), 5000);
    } else {
        connect(modbusDevice, &QModbusClient::stateChanged,
                this, &ModbusUi::onStateChanged);
    }
}
void ModbusUi::onStateChanged(int state)
{
//    bool connected = (state != QModbusDevice::UnconnectedState);
//    ui->actionConnect->setEnabled(!connected);
//    ui->actionDisconnect->setEnabled(connected);

    if (state == QModbusDevice::UnconnectedState)
        ui->connectButton->setText(tr("   Connect   "));
    else if (state == QModbusDevice::ConnectedState)
        ui->connectButton->setText(tr("  Disconnect "));
}

void ModbusUi::on_connectButton_clicked()
{
    if (!modbusDevice)
        return;

//    statusBar()->clearMessage();
    if (modbusDevice->state() != QModbusDevice::ConnectedState) {
        if (static_cast<ModbusConnection> (ui->connectType->currentIndex()) == Serial) {
            modbusDevice->setConnectionParameter(QModbusDevice::SerialPortNameParameter,
                ui->serialPortInfoListBox->currentText());

            modbusDevice->setConnectionParameter(QModbusDevice::SerialParityParameter,
                static_cast<QSerialPort::Parity>(
                     ui->parityCombo->itemData(ui->parityCombo->currentIndex()).toInt()));

            if (ui->baudCombo->currentIndex() == 4) {
            modbusDevice->setConnectionParameter(QModbusDevice::SerialBaudRateParameter,
                ui->baudCombo->currentText().toInt());
            }else{
            modbusDevice->setConnectionParameter(QModbusDevice::SerialBaudRateParameter,
                static_cast<QSerialPort::BaudRate>(
                     ui->baudCombo->itemData(ui->baudCombo->currentIndex()).toInt()));
            }

            modbusDevice->setConnectionParameter(QModbusDevice::SerialDataBitsParameter,
                static_cast<QSerialPort::DataBits>(
                     ui->dataBitsCombo->itemData(ui->dataBitsCombo->currentIndex()).toInt()));

            modbusDevice->setConnectionParameter(QModbusDevice::SerialStopBitsParameter,
                static_cast<QSerialPort::StopBits>(
                     ui->stopBitsCombo->itemData(ui->stopBitsCombo->currentIndex()).toInt()));

            modbusDevice->setConnectionParameter(QModbusDevice::SerialStopBitsParameter,
                static_cast<QSerialPort::StopBits>(
                     ui->stopBitsCombo->itemData(ui->stopBitsCombo->currentIndex()).toInt()));
        }
/*        if (static_cast<ModbusConnection> (ui->connectType->currentIndex()) == Serial) {
            modbusDevice->setConnectionParameter(QModbusDevice::SerialPortNameParameter,
                ui->serialPortInfoListBox->currentText());
            modbusDevice->setConnectionParameter(QModbusDevice::SerialParityParameter,
                ui->parityCombo->currentIndex());
            modbusDevice->setConnectionParameter(QModbusDevice::SerialBaudRateParameter,
                ui->baudCombo->currentText().toInt());
            modbusDevice->setConnectionParameter(QModbusDevice::SerialDataBitsParameter,
                ui->dataBitsCombo->currentText().toInt());
            modbusDevice->setConnectionParameter(QModbusDevice::SerialStopBitsParameter,
                ui->stopBitsCombo->currentText().toInt());
        }*/else {
            const QUrl url = QUrl::fromUserInput(ui->portEdit->text());
            modbusDevice->setConnectionParameter(QModbusDevice::NetworkPortParameter, url.port());
            modbusDevice->setConnectionParameter(QModbusDevice::NetworkAddressParameter, url.host());
        }
        modbusDevice->setTimeout(ui->timeoutSpinner->value());
        modbusDevice->setNumberOfRetries(ui->retriesSpinner->value());
        if (!modbusDevice->connectDevice()) {
            qDebug() << "Connect failed:" <<modbusDevice->errorString() ;

//            statusBar()->showMessage(tr("Connect failed: ") + modbusDevice->errorString(), 5000);
        } else {
            qDebug() << "connect ok";
//            ui->actionConnect->setEnabled(false);
//            ui->actionDisconnect->setEnabled(true);
        }
    } else {
        modbusDevice->disconnectDevice();
//        ui->actionConnect->setEnabled(true);
//        ui->actionDisconnect->setEnabled(false);
    }
}


ModbusUi::~ModbusUi()
{
    qDebug("ModbusUi exit");
    if (modbusDevice)
        modbusDevice->disconnectDevice();

    instance = Q_NULLPTR;
    delete ui;
}

ModbusUi *ModbusUi::getInstance()
{
    if(!instance)
    {
        instance = new ModbusUi();
    }
    return instance;
}

void ModbusUi::deleteInstance()
{
    if(instance)
        instance->deleteLater();
}

QModbusClient* ModbusUi::getModbusDevice()
{
    return modbusDevice;
}

void ModbusUi::fillPortsInfo()
{
    ui->serialPortInfoListBox->clear();
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        QStringList list;
        list << info.portName();
        ui->serialPortInfoListBox->addItem(list.first(), list);
    }

    ui->serialPortInfoListBox->addItem(tr("Custom"));
}

void ModbusUi::on_searchButton_clicked()
{
    fillPortsInfo();
    ui->serialPortInfoListBox->showPopup();
}
void ModbusUi::checkCustomBaudRatePolicy(int idx)
{
    bool isCustomBaudRate = !ui->baudCombo->itemData(idx).isValid();
    ui->baudCombo->setEditable(isCustomBaudRate);
    if (isCustomBaudRate) {
        ui->baudCombo->clearEditText();
        QLineEdit *edit = ui->baudCombo->lineEdit();
        edit->setValidator(intValidator);
    }
}

void ModbusUi::checkCustomDevicePathPolicy(int idx)
{
    bool isCustomPath = !ui->serialPortInfoListBox->itemData(idx).isValid();
    ui->serialPortInfoListBox->setEditable(isCustomPath);
    if (isCustomPath)
        ui->serialPortInfoListBox->clearEditText();
}

void ModbusUi::fillPortsParameters()
{
    ui->baudCombo->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
    ui->baudCombo->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
    ui->baudCombo->addItem(QStringLiteral("38400"), QSerialPort::Baud38400);
    ui->baudCombo->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);
    ui->baudCombo->addItem(tr("Custom"));
    ui->baudCombo->setCurrentIndex(0);

    ui->dataBitsCombo->addItem(QStringLiteral("5"), QSerialPort::Data5);
    ui->dataBitsCombo->addItem(QStringLiteral("6"), QSerialPort::Data6);
    ui->dataBitsCombo->addItem(QStringLiteral("7"), QSerialPort::Data7);
    ui->dataBitsCombo->addItem(QStringLiteral("8"), QSerialPort::Data8);
    ui->dataBitsCombo->setCurrentIndex(3);

    ui->parityCombo->addItem(tr("None"), QSerialPort::NoParity);
    ui->parityCombo->addItem(tr("Even"), QSerialPort::EvenParity);
    ui->parityCombo->addItem(tr("Odd"), QSerialPort::OddParity);
    ui->parityCombo->addItem(tr("Mark"), QSerialPort::MarkParity);
    ui->parityCombo->addItem(tr("Space"), QSerialPort::SpaceParity);
    ui->parityCombo->setCurrentIndex(0);

    ui->stopBitsCombo->addItem(QStringLiteral("1"), QSerialPort::OneStop);
#ifdef Q_OS_WIN
    ui->stopBitsCombo->addItem(tr("1.5"), QSerialPort::OneAndHalfStop);
#endif
    ui->stopBitsCombo->addItem(QStringLiteral("2"), QSerialPort::TwoStop);

}

void ModbusUi::on_readButton_clicked()
{
    if (!modbusDevice)
        return;
    ui->readValue->clear();
//    statusBar()->clearMessage();
qDebug() <<"read";
    if (auto *reply = modbusDevice->sendReadRequest(readRequest(), ui->serverEdit->value())) {
        if (!reply->isFinished()){
            connect(reply, &QModbusReply::finished, this, &ModbusUi::readReady);
            qDebug() <<"wait";
        }
        else
            delete reply; // broadcast replies return immediately
    } else {
        qDebug() << "error:" <<modbusDevice->errorString() ;
//        statusBar()->showMessage(tr("Read error: ") + modbusDevice->errorString(), 5000);
    }
}

void ModbusUi::readReady()
{
     qDebug() <<"readready";
    QModbusReply *reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;

    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = reply->result();
        for (uint i = 0; i < unit.valueCount(); i++) {
            const QString entry = tr("Address: 0x%1, Value: %2").arg(QString::number(unit.startAddress()+i, 16))
                                     .arg(QString::number(unit.value(i),
                                          unit.registerType() <= QModbusDataUnit::Coils ? 10 : 16));
            ui->readValue->addItem(entry);
        }
    } else if (reply->error() == QModbusDevice::ProtocolError) {
        qDebug() <<reply->errorString();
//        statusBar()->showMessage(tr("Read response error: %1 (Mobus exception: 0x%2)").
//                                    arg(reply->errorString()).
//                                    arg(reply->rawResult().exceptionCode(), -1, 16), 5000);
    } else {
        qDebug() <<reply->errorString();
//        statusBar()->showMessage(tr("Read response error: %1 (code: 0x%2)").
//                                    arg(reply->errorString()).
//                                    arg(reply->error(), -1, 16), 5000);
    }

    reply->deleteLater();
}

void ModbusUi::on_writeButton_clicked()
{
    if (!modbusDevice)
        return;
//    statusBar()->clearMessage();

    QModbusDataUnit writeUnit = writeRequest();
    QModbusDataUnit::RegisterType table = writeUnit.registerType();
    for (uint i = 0; i < writeUnit.valueCount(); i++) {
        if (table == QModbusDataUnit::Coils)
            writeUnit.setValue(i, writeModel->m_coils[i]);
        else
            writeUnit.setValue(i, writeModel->m_holdingRegisters[i]);
    }

    if (auto *reply = modbusDevice->sendWriteRequest(writeUnit, ui->serverEdit->value())) {
        if (!reply->isFinished()) {
            connect(reply, &QModbusReply::finished, this, [this, reply]() {
                if (reply->error() == QModbusDevice::ProtocolError) {
//                    statusBar()->showMessage(tr("Write response error: %1 (Mobus exception: 0x%2)")
//                        .arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 16),
//                        5000);
                } else if (reply->error() != QModbusDevice::NoError) {
//                    statusBar()->showMessage(tr("Write response error: %1 (code: 0x%2)").
//                        arg(reply->errorString()).arg(reply->error(), -1, 16), 5000);
                }
                reply->deleteLater();
            });
        } else {
            // broadcast replies return immediately
            reply->deleteLater();
        }
    } else {
//        statusBar()->showMessage(tr("Write error: ") + modbusDevice->errorString(), 5000);
    }
}

void ModbusUi::on_readWriteButton_clicked()
{
    if (!modbusDevice)
        return;
    ui->readValue->clear();
//    statusBar()->clearMessage();

    QModbusDataUnit writeUnit = writeRequest();
    QModbusDataUnit::RegisterType table = writeUnit.registerType();
    for (uint i = 0; i < writeUnit.valueCount(); i++) {
        if (table == QModbusDataUnit::Coils)
            writeUnit.setValue(i, writeModel->m_coils[i]);
        else
            writeUnit.setValue(i, writeModel->m_holdingRegisters[i]);
    }

    if (auto *reply = modbusDevice->sendReadWriteRequest(readRequest(), writeUnit,
        ui->serverEdit->value())) {
        if (!reply->isFinished())
            connect(reply, &QModbusReply::finished, this, &ModbusUi::readReady);
        else
            delete reply; // broadcast replies return immediately
    } else {
//        statusBar()->showMessage(tr("Read error: ") + modbusDevice->errorString(), 5000);
    }
}

void ModbusUi::on_writeTable_currentIndexChanged(int index)
{
    const bool coilsOrHolding = index == 0 || index == 3;
    if (coilsOrHolding) {
        ui->writeValueTable->setColumnHidden(1, index != 0);
        ui->writeValueTable->setColumnHidden(2, index != 3);
        ui->writeValueTable->resizeColumnToContents(0);
    }

    ui->readWriteButton->setEnabled(index == 3);
    ui->writeButton->setEnabled(coilsOrHolding);
    ui->writeGroupBox->setEnabled(coilsOrHolding);
}

QModbusDataUnit ModbusUi::readRequest() const
{
    const QModbusDataUnit::RegisterType table =
        static_cast<QModbusDataUnit::RegisterType> (ui->writeTable->currentData().toInt());

    int startAddress = ui->readAddress->value();
    Q_ASSERT(startAddress >= 0 && startAddress < ROW_COUNT);

    int numberOfEntries = ui->readSize->currentText().toInt();
    return QModbusDataUnit(table, startAddress, numberOfEntries);
}

QModbusDataUnit ModbusUi::writeRequest() const
{
    const QModbusDataUnit::RegisterType table =
        static_cast<QModbusDataUnit::RegisterType> (ui->writeTable->currentData().toInt());

    int startAddress = ui->writeAddress->value();
    Q_ASSERT(startAddress >= 0 && startAddress < ROW_COUNT);

    int numberOfEntries = ui->writeSize->currentText().toInt();
    return QModbusDataUnit(table, startAddress, numberOfEntries);
}
