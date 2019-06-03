#include "quamodbusclientwidgetedit.h"
#include "ui_quamodbusclientwidgetedit.h"

#include <QMetaEnum>

#include <QUaModbusRtuSerialClient>

QUaModbusClientWidgetEdit::QUaModbusClientWidgetEdit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QUaModbusClientWidgetEdit)
{
    ui->setupUi(this);
	// setup type combo
	for (int i = (int)ClientType::Tcp; i < (int)ClientType::Invalid; i++)
	{
		auto strType = QString(QMetaEnum::fromType<ClientType>().valueToKey(i));
		ui->comboBoxType->addItem(strType, i);
	}
	// setup com port combo
	auto mapComPorts = QUaModbusRtuSerialClient::EnumComPorts();
	for (int i = 0; i < mapComPorts.keys().count(); i++)
	{
		auto strComPort = QString(mapComPorts.value(mapComPorts.keys().at(i)));
		ui->comboBoxComPort->addItem(strComPort, mapComPorts.keys().at(i));
	}
	// setup parity combo
	auto metaParity = QMetaEnum::fromType<QParity>();
	for (int i = 0; i < metaParity.keyCount(); i++)
	{
		auto enumParity = metaParity.value(i);
		auto strParity  = QString(metaParity.key(i));
		ui->comboBoxParity->addItem(strParity, enumParity);
	}
	// setup baud rate combo
	auto metaBaudRate = QMetaEnum::fromType<QBaudRate>();
	for (int i = 0; i < metaBaudRate.keyCount(); i++)
	{
		auto enumBaudRate = metaBaudRate.value(i);
		auto strBaudRate  = QString(metaBaudRate.key(i));
		ui->comboBoxBaudRate->addItem(strBaudRate, enumBaudRate);
	}
	// setup data bits combo
	auto metaDataBits = QMetaEnum::fromType<QDataBits>();
	for (int i = 0; i < metaDataBits.keyCount(); i++)
	{
		auto enumDataBits = metaDataBits.value(i);
		auto strDataBits  = QString(metaDataBits.key(i));
		ui->comboBoxDataBits->addItem(strDataBits, enumDataBits);
	}
	// setup stop bits combo
	auto metaStopBits = QMetaEnum::fromType<QStopBits>();
	for (int i = 0; i < metaStopBits.keyCount(); i++)
	{
		auto enumStopBits = metaStopBits.value(i);
		auto strStopBits  = QString(metaStopBits.key(i));
		ui->comboBoxStopBits->addItem(strStopBits, enumStopBits);
	}
	// setup initial values
	this->setType(ClientType::Tcp);
	this->setDeviceAddress(0);
	this->setNetworkPort(502);
	this->setKeepConnecting(false);
	this->setIpAddress("127.0.0.1");
	if (mapComPorts.count() > 0)
	{
		this->setComPort(QString(mapComPorts.value(mapComPorts.keys().at(0))));
	}
	this->setParity(QParity::EvenParity);
	this->setBaudRate(QBaudRate::Baud19200);
	this->setDataBits(QSerialPort::Data8);
	this->setStopBits(QSerialPort::OneStop);
}

QUaModbusClientWidgetEdit::~QUaModbusClientWidgetEdit()
{
    delete ui;
}

bool QUaModbusClientWidgetEdit::isIdEditable() const
{
	return ui->lineEditId->isEnabled();
}

void QUaModbusClientWidgetEdit::setIdEditable(const bool & idEditable)
{
	ui->lineEditId->setEnabled(idEditable);
}

bool QUaModbusClientWidgetEdit::isTypeEditable() const
{
	return ui->comboBoxType->isEnabled();
}

void QUaModbusClientWidgetEdit::setTypeEditable(const bool & typeEditable)
{
	ui->comboBoxType->setEnabled(typeEditable);
}

QString QUaModbusClientWidgetEdit::id() const
{
	return ui->lineEditId->text();
}

void QUaModbusClientWidgetEdit::setId(const QString & strId)
{
	ui->lineEditId->setText(strId);
}

QUaModbusClientWidgetEdit::ClientType QUaModbusClientWidgetEdit::type() const
{
	ClientType clientType = ui->comboBoxType->currentData().value<ClientType>();
	return clientType;
}

void QUaModbusClientWidgetEdit::setType(const ClientType & type)
{
	auto strType = QString(QMetaEnum::fromType<ClientType>().valueToKey(type));
	Q_ASSERT(ui->comboBoxType->findText(strType) >= 0);
	ui->comboBoxType->setCurrentText(strType);
	// update ui
	this->updateTypeInGui(type);
}

quint8 QUaModbusClientWidgetEdit::deviceAddress() const
{
	return ui->spinBoxDeviceAddress->value();
}

void QUaModbusClientWidgetEdit::setDeviceAddress(const quint8 & deviceAddress)
{
	ui->spinBoxDeviceAddress->setValue(deviceAddress);
}

quint16 QUaModbusClientWidgetEdit::networkPort() const
{
	return ui->spinBoxPort->value();
}

void QUaModbusClientWidgetEdit::setNetworkPort(const quint16 & networkPort)
{
	ui->spinBoxPort->setValue(networkPort);
}

bool QUaModbusClientWidgetEdit::keepConnecting() const
{
	return ui->checkBoxKeepConnect->isChecked();
}

void QUaModbusClientWidgetEdit::setKeepConnecting(const bool & keepConnecting)
{
	ui->checkBoxKeepConnect->setChecked(keepConnecting);
}

QString QUaModbusClientWidgetEdit::ipAddress() const
{
	return ui->lineEditAddress->text();
}

void QUaModbusClientWidgetEdit::setIpAddress(const QString & strIpAddress)
{
	ui->lineEditAddress->setText(strIpAddress);
}

QString QUaModbusClientWidgetEdit::comPort() const
{
	return ui->comboBoxComPort->currentText();
}

void QUaModbusClientWidgetEdit::setComPort(const QString & strComPort)
{
	ui->comboBoxComPort->setCurrentText(strComPort);
}

int QUaModbusClientWidgetEdit::comPortKey() const
{
	auto strComPort  = this->comPort();
	auto mapComPorts = QUaModbusRtuSerialClient::EnumComPorts();
	return mapComPorts.key(strComPort.toUtf8(), -1);
}

void QUaModbusClientWidgetEdit::setComPortKey(const int & comPortKey)
{
	auto mapComPorts = QUaModbusRtuSerialClient::EnumComPorts();
	this->setComPort(mapComPorts.value(comPortKey));
}

QParity QUaModbusClientWidgetEdit::parity() const
{
	return ui->comboBoxParity->currentData().value<QParity>();
}

void QUaModbusClientWidgetEdit::setParity(const QParity & parity)
{
	auto strParity = QString(QMetaEnum::fromType<QParity>().valueToKey(parity));
	Q_ASSERT(ui->comboBoxParity->findText(strParity) >= 0);
	ui->comboBoxParity->setCurrentText(strParity);
}

QBaudRate QUaModbusClientWidgetEdit::baudRate() const
{
	return ui->comboBoxBaudRate->currentData().value<QBaudRate>();
}

void QUaModbusClientWidgetEdit::setBaudRate(const QBaudRate & baudRate)
{
	auto strBaudRate = QString(QMetaEnum::fromType<QBaudRate>().valueToKey(baudRate));
	Q_ASSERT(ui->comboBoxBaudRate->findText(strBaudRate) >= 0);
	ui->comboBoxBaudRate->setCurrentText(strBaudRate);
}

QDataBits QUaModbusClientWidgetEdit::dataBits() const
{
	return ui->comboBoxDataBits->currentData().value<QDataBits>();
}

void QUaModbusClientWidgetEdit::setDataBits(const QDataBits & dataBits)
{
	auto strDataBits = QString(QMetaEnum::fromType<QDataBits>().valueToKey(dataBits));
	Q_ASSERT(ui->comboBoxDataBits->findText(strDataBits) >= 0);
	ui->comboBoxDataBits->setCurrentText(strDataBits);
}

QStopBits QUaModbusClientWidgetEdit::stopBits() const
{
	return ui->comboBoxStopBits->currentData().value<QStopBits>();
}

void QUaModbusClientWidgetEdit::setStopBits(const QStopBits & stopBits)
{
	auto strStopBits = QString(QMetaEnum::fromType<QStopBits>().valueToKey(stopBits));
	Q_ASSERT(ui->comboBoxStopBits->findText(strStopBits) >= 0);
	ui->comboBoxStopBits->setCurrentText(strStopBits);
}

void QUaModbusClientWidgetEdit::updateTypeInGui(const ClientType & type)
{
	// display only relevant frame
	switch (type)
	{
	case ClientType::Tcp:
		ui->frameSerial->setEnabled(false);
		ui->frameSerial->setVisible(false);
		ui->frameTcp->setEnabled(true);
		ui->frameTcp->setVisible(true);
		break;
	case ClientType::Serial:
		ui->frameSerial->setEnabled(true);
		ui->frameSerial->setVisible(true);
		ui->frameTcp->setEnabled(false);
		ui->frameTcp->setVisible(false);
		break;
	default:
		Q_ASSERT(false);
		break;
	}
}

void QUaModbusClientWidgetEdit::on_comboBoxType_currentIndexChanged(int index)
{
	Q_UNUSED(index);
	ClientType type = this->type();
	this->updateTypeInGui(type);
}
