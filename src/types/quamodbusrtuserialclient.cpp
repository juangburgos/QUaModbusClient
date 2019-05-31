#include "quamodbusrtuserialclient.h"

#include <QSerialPortInfo>

QUaModbusRtuSerialClient::QUaModbusRtuSerialClient(QUaServer *server)
	: QUaModbusClient(server)
{
	// set defaults
	type    ()->setValue("Serial");
	comPort ()->setDataTypeEnum(QUaModbusRtuSerialClient::ComPorts);
	comPort ()->setValue(0);
	parity  ()->setDataTypeEnum(QMetaEnum::fromType<QParity>());
	parity  ()->setValue(QSerialPort::EvenParity);
	baudRate()->setDataTypeEnum(QMetaEnum::fromType<QBaudRate>());
	baudRate()->setValue(QSerialPort::Baud19200);
	dataBits()->setDataTypeEnum(QMetaEnum::fromType<QDataBits>());
	dataBits()->setValue(QSerialPort::Data8);
	stopBits()->setDataTypeEnum(QMetaEnum::fromType<QStopBits>());
	stopBits()->setValue(QSerialPort::OneStop);
	// set initial conditions
	comPort ()->setWriteAccess(true);
	parity  ()->setWriteAccess(true);
	baudRate()->setWriteAccess(true);
	dataBits()->setWriteAccess(true);
	stopBits()->setWriteAccess(true);
	// instantiate client
	m_workerThread.execInThread([this]() {
		// instantiate in thread so it runs on the thread
		m_modbusClient.reset(new QModbusRtuSerialMaster(nullptr), [](QObject * client) {
			client->deleteLater();
		});
		// defaults
		m_modbusClient->setConnectionParameter(QModbusDevice::SerialPortNameParameter, QString(QUaModbusRtuSerialClient::EnumComPorts().value(0)));
		m_modbusClient->setConnectionParameter(QModbusDevice::SerialParityParameter  , QSerialPort::EvenParity);
		m_modbusClient->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, QSerialPort::Baud19200 );
		m_modbusClient->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, QSerialPort::Data8     );
		m_modbusClient->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, QSerialPort::OneStop   );
		// setup client (call base class method)
		this->setupModbusClient();
		QObject::connect(m_modbusClient.data(), &QModbusClient::stateChanged, this, &QUaModbusRtuSerialClient::on_stateChanged, Qt::QueuedConnection);
	});
	// handle state changes
	QObject::connect(comPort (), &QUaBaseVariable::valueChanged, this, &QUaModbusRtuSerialClient::on_comPortChanged , Qt::QueuedConnection);
	QObject::connect(parity  (), &QUaBaseVariable::valueChanged, this, &QUaModbusRtuSerialClient::on_parityChanged  , Qt::QueuedConnection);
	QObject::connect(baudRate(), &QUaBaseVariable::valueChanged, this, &QUaModbusRtuSerialClient::on_baudRateChanged, Qt::QueuedConnection);
	QObject::connect(dataBits(), &QUaBaseVariable::valueChanged, this, &QUaModbusRtuSerialClient::on_dataBitsChanged, Qt::QueuedConnection);
	QObject::connect(stopBits(), &QUaBaseVariable::valueChanged, this, &QUaModbusRtuSerialClient::on_stopBitsChanged, Qt::QueuedConnection);
	// set descriptions
	comPort ()->setDescription("Local serial COM port used to connect to the Modbus server.");
	parity  ()->setDescription("Parity value (for error detection) used to communicate with the Modbus server.");
	baudRate()->setDescription("Baud Rate value (data rate in bits per second) used to communicate with the Modbus server.");
	dataBits()->setDescription("Number of Data Bits (in each character) used to communicate with the Modbus server.");
	stopBits()->setDescription("Number of Stop Bits (sent at the end of every character) used to communicate with the Modbus server.");
}

QUaProperty * QUaModbusRtuSerialClient::comPort() const
{
	return this->browseChild<QUaProperty>("ComPort");
}

QUaProperty * QUaModbusRtuSerialClient::parity() const
{
	return this->browseChild<QUaProperty>("Parity");
}

QUaProperty * QUaModbusRtuSerialClient::baudRate() const
{
	return this->browseChild<QUaProperty>("BaudRate");
}

QUaProperty * QUaModbusRtuSerialClient::dataBits() const
{
	return this->browseChild<QUaProperty>("DataBits");
}

QUaProperty * QUaModbusRtuSerialClient::stopBits() const
{
	return this->browseChild<QUaProperty>("StopBits");
}

QString QUaModbusRtuSerialClient::ComPorts = "QUaModbusRtuSerialClient::ComPorts";

QMap<int, QByteArray> QUaModbusRtuSerialClient::EnumComPorts()
{
	QMap<int, QByteArray> mapPorts;
	QList<QSerialPortInfo> list = QSerialPortInfo::availablePorts();
	for (int i = 0; i < list.count(); i++)
	{
		QSerialPortInfo portInfo = list.at(i);
		mapPorts.insert(i, portInfo.portName().toUtf8());
	}
	return mapPorts;
}

QDomElement QUaModbusRtuSerialClient::toDomElement(QDomDocument & domDoc) const
{
	// add client list element
	QDomElement elemSerialClient = domDoc.createElement(QUaModbusRtuSerialClient::staticMetaObject.className());
	// set client attributes
	elemSerialClient.setAttribute("BrowseName"    , this->browseName());
	elemSerialClient.setAttribute("ServerAddress" , serverAddress()->value().toUInt());
	elemSerialClient.setAttribute("KeepConnecting", keepConnecting()->value().toBool());
	elemSerialClient.setAttribute("ComPort"       , QString(QUaModbusRtuSerialClient::EnumComPorts().value(comPort()->value().toInt())));
	elemSerialClient.setAttribute("Parity"  , QMetaEnum::fromType<QParity>  ().valueToKey(parity  ()->value().value<QParity>  ()));
	elemSerialClient.setAttribute("BaudRate", QMetaEnum::fromType<QBaudRate>().valueToKey(baudRate()->value().value<QBaudRate>()));
	elemSerialClient.setAttribute("DataBits", QMetaEnum::fromType<QDataBits>().valueToKey(dataBits()->value().value<QDataBits>()));
	elemSerialClient.setAttribute("StopBits", QMetaEnum::fromType<QStopBits>().valueToKey(stopBits()->value().value<QStopBits>()));
	// add block list element
	auto elemBlockList = dataBlocks()->toDomElement(domDoc);
	elemSerialClient.appendChild(elemBlockList);
	// return list element
	return elemSerialClient;
}

void QUaModbusRtuSerialClient::fromDomElement(QDomElement & domElem, QString & strError)
{
	// get client attributes (BrowseName must be already set)
	QString strBrowseName = domElem.attribute("BrowseName", "");
	Q_ASSERT(browseName().compare(strBrowseName, Qt::CaseInsensitive) == 0);
	bool bOK;
	// ServerAddress
	auto serverAddress = domElem.attribute("ServerAddress").toUInt(&bOK);
	if (bOK)
	{
		this->serverAddress()->setValue(serverAddress);
	}
	else
	{
		strError += QString("Error : Invalid ServerAddress attribute '%1' in Modbus client %2. Ignoring.\n").arg(serverAddress).arg(strBrowseName);
	}
	// KeepConnecting
	auto keepConnecting = (bool)domElem.attribute("KeepConnecting").toUInt(&bOK);
	if (bOK)
	{
		this->keepConnecting()->setValue(keepConnecting);
	}
	else
	{
		strError += QString("Error : Invalid KeepConnecting attribute '%1' in Modbus client %2. Ignoring.\n").arg(keepConnecting).arg(strBrowseName);
	}
	// ComPort
	auto comPort = domElem.attribute("ComPort");
	if (!comPort.isEmpty())
	{
		this->comPort()->setValue(QUaModbusRtuSerialClient::EnumComPorts().key(comPort.toUtf8(), 0));
		// NOTE : force internal update (if connected won't apply until reconnect)
		this->on_comPortChanged(comPort);
	}
	else
	{
		strError += QString("Error : Invalid ComPort attribute '%1' in Modbus client %2. Ignoring.\n").arg(comPort).arg(strBrowseName);
	}
	// Parity
	auto parity = QMetaEnum::fromType<QParity>().keysToValue(domElem.attribute("Parity").toUtf8(), &bOK);
	if (bOK)
	{
		this->parity()->setValue(parity);
		// NOTE : force internal update (if connected won't apply until reconnect)
		this->on_parityChanged(parity);
	}
	else
	{
		strError += QString("Error : Invalid Parity attribute '%1' in Modbus client %2. Ignoring.\n").arg(parity).arg(strBrowseName);
	}
	// BaudRate
	auto baudRate = QMetaEnum::fromType<QBaudRate>().keysToValue(domElem.attribute("BaudRate").toUtf8(), &bOK);
	if (bOK)
	{
		this->baudRate()->setValue(baudRate);
		// NOTE : force internal update (if connected won't apply until reconnect)
		this->on_baudRateChanged(baudRate);
	}
	else
	{
		strError += QString("Error : Invalid BaudRate attribute '%1' in Modbus client %2. Ignoring.\n").arg(baudRate).arg(strBrowseName);
	}
	// DataBits
	auto dataBits = QMetaEnum::fromType<QDataBits>().keysToValue(domElem.attribute("DataBits").toUtf8(), &bOK);
	if (bOK)
	{
		this->dataBits()->setValue(dataBits);
		// NOTE : force internal update (if connected won't apply until reconnect)
		this->on_dataBitsChanged(dataBits);
	}
	else
	{
		strError += QString("Error : Invalid DataBits attribute '%1' in Modbus client %2. Ignoring.\n").arg(dataBits).arg(strBrowseName);
	}
	// StopBits
	auto stopBits = QMetaEnum::fromType<QStopBits>().keysToValue(domElem.attribute("StopBits").toUtf8(), &bOK);
	if (bOK)
	{
		this->stopBits()->setValue(stopBits);
		// NOTE : force internal update (if connected won't apply until reconnect)
		this->on_stopBitsChanged(stopBits);
	}
	else
	{
		strError += QString("Error : Invalid StopBits attribute '%1' in Modbus client %2. Ignoring.\n").arg(stopBits).arg(strBrowseName);
	}
	// get block list
	QDomElement elemBlockList = domElem.firstChildElement(QUaModbusDataBlockList::staticMetaObject.className());
	if (!elemBlockList.isNull())
	{
		dataBlocks()->fromDomElement(elemBlockList, strError);
	}
	else
	{
		strError += QString("Error : Modbus client %1 does not have a QUaModbusDataBlockList child. No blocks will be loaded.\n").arg(strBrowseName);
	}
}

void QUaModbusRtuSerialClient::on_stateChanged(const QModbusDevice::State &state)
{
	// only allow to write connection params if not connected
	if (state == QModbusDevice::State::UnconnectedState)
	{
		comPort ()->setWriteAccess(true);
		parity  ()->setWriteAccess(true);
		baudRate()->setWriteAccess(true);
		dataBits()->setWriteAccess(true);
		stopBits()->setWriteAccess(true);
	}
	else
	{
		comPort ()->setWriteAccess(false);
		parity  ()->setWriteAccess(false);
		baudRate()->setWriteAccess(false);
		dataBits()->setWriteAccess(false);
		stopBits()->setWriteAccess(false);
	}
}

void QUaModbusRtuSerialClient::on_comPortChanged(const QVariant & value)
{
	// NOTE : if connected, will not change until reconnect
	QString strComPort = QUaModbusRtuSerialClient::EnumComPorts().value(value.toInt());
	// set in thread, for thread-safety
	m_workerThread.execInThread([this, strComPort]() {
		m_modbusClient->setConnectionParameter(QModbusDevice::SerialPortNameParameter, strComPort);
	});
	// emit
	emit this->comPortChanged(strComPort);
}

void QUaModbusRtuSerialClient::on_parityChanged(const QVariant & value)
{
	// NOTE : if connected, will not change until reconnect
	QParity parity = value.value<QParity>();
	// set in thread, for thread-safety
	m_workerThread.execInThread([this, parity]() {
		m_modbusClient->setConnectionParameter(QModbusDevice::SerialParityParameter, parity);
	});
	// emit
	emit this->parityChanged(parity);
}

void QUaModbusRtuSerialClient::on_baudRateChanged(const QVariant & value)
{
	// NOTE : if connected, will not change until reconnect
	QBaudRate baudRate = value.value<QBaudRate>();
	// set in thread, for thread-safety
	m_workerThread.execInThread([this, baudRate]() {
		m_modbusClient->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, baudRate);
	});
	// emit
	emit this->baudRateChanged(baudRate);
}

void QUaModbusRtuSerialClient::on_dataBitsChanged(const QVariant & value)
{
	// NOTE : if connected, will not change until reconnect
	QDataBits dataBits = value.value<QDataBits>();
	// set in thread, for thread-safety
	m_workerThread.execInThread([this, dataBits]() {
		m_modbusClient->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, dataBits);
	});
	// emit
	emit this->dataBitsChanged(dataBits);
}

void QUaModbusRtuSerialClient::on_stopBitsChanged(const QVariant & value)
{
	// NOTE : if connected, will not change until reconnect
	QStopBits stopBits = value.value<QStopBits>();
	// set in thread, for thread-safety
	m_workerThread.execInThread([this, stopBits]() {
		m_modbusClient->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, stopBits);
	});
	// emit
	emit this->stopBitsChanged(stopBits);
}

QString QUaModbusRtuSerialClient::getComPort() const
{
	auto key = this->comPort()->value().toInt();
	return QUaModbusRtuSerialClient::EnumComPorts().value(key);
}

void QUaModbusRtuSerialClient::setComPort(const QString & strComPort)
{
	auto comPort = QUaModbusRtuSerialClient::EnumComPorts().key(strComPort.toUtf8(), 0);
	this->comPort()->setValue(comPort);
	this->on_comPortChanged(comPort);
}

int QUaModbusRtuSerialClient::getComPortKey() const
{
	return this->comPort()->value().toInt();
}

void QUaModbusRtuSerialClient::setComPortKey(const int & comPort)
{
	this->comPort()->setValue(comPort);
	this->on_comPortChanged(comPort);
}

QParity QUaModbusRtuSerialClient::getParity() const
{
	return this->parity()->value().value<QParity>();
}

void QUaModbusRtuSerialClient::setParity(const QParity & parity)
{
	this->parity()->setValue(parity);
	this->on_parityChanged(parity);
}

QBaudRate QUaModbusRtuSerialClient::getBaudRate() const
{
	return this->baudRate()->value().value<QBaudRate>();
}

void QUaModbusRtuSerialClient::setBaudRate(const QBaudRate & baudRate)
{
	this->baudRate()->setValue(baudRate);
	this->on_baudRateChanged(baudRate);
}

QDataBits QUaModbusRtuSerialClient::getDataBits() const
{
	return this->dataBits()->value().value<QDataBits>();
}

void QUaModbusRtuSerialClient::setDataBits(const QDataBits & dataBits)
{
	this->dataBits()->setValue(dataBits);
	this->on_dataBitsChanged(dataBits);
}

QStopBits QUaModbusRtuSerialClient::getStopBits() const
{
	return this->stopBits()->value().value<QStopBits>();
}

void QUaModbusRtuSerialClient::setStopBits(const QStopBits & stopBits)
{
	this->stopBits()->setValue(stopBits);
	this->on_stopBitsChanged(stopBits);
}
