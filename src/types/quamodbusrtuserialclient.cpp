#include "quamodbusrtuserialclient.h"

#include <QSerialPortInfo>

#ifdef QUA_ACCESS_CONTROL
#include <QUaPermissions>
#endif // QUA_ACCESS_CONTROL

QUaModbusRtuSerialClient::QUaModbusRtuSerialClient(QUaServer *server)
	: QUaModbusClient(server)
{
	// set defaults
	type    ()->setDataTypeEnum(QMetaEnum::fromType<QModbusClientType>());
	type    ()->setValue(QModbusClientType::Serial);
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
		m_modbusClient->setConnectionParameter(QModbusDevice::SerialPortNameParameter, QString(QUaModbusRtuSerialClient::EnumComPorts().value(0).displayName.text()));
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
	/*
	comPort ()->setDescription("Local serial COM port used to connect to the Modbus server.");
	parity  ()->setDescription("Parity value (for error detection) used to communicate with the Modbus server.");
	baudRate()->setDescription("Baud Rate value (data rate in bits per second) used to communicate with the Modbus server.");
	dataBits()->setDescription("Number of Data Bits (in each character) used to communicate with the Modbus server.");
	stopBits()->setDescription("Number of Stop Bits (sent at the end of every character) used to communicate with the Modbus server.");
	*/
}

QUaProperty * QUaModbusRtuSerialClient::comPort() const
{
	return const_cast<QUaModbusRtuSerialClient*>(this)->browseChild<QUaProperty>("ComPort");
}

QUaProperty * QUaModbusRtuSerialClient::parity() const
{
	return const_cast<QUaModbusRtuSerialClient*>(this)->browseChild<QUaProperty>("Parity");
}

QUaProperty * QUaModbusRtuSerialClient::baudRate() const
{
	return const_cast<QUaModbusRtuSerialClient*>(this)->browseChild<QUaProperty>("BaudRate");
}

QUaProperty * QUaModbusRtuSerialClient::dataBits() const
{
	return const_cast<QUaModbusRtuSerialClient*>(this)->browseChild<QUaProperty>("DataBits");
}

QUaProperty * QUaModbusRtuSerialClient::stopBits() const
{
	return const_cast<QUaModbusRtuSerialClient*>(this)->browseChild<QUaProperty>("StopBits");
}

QString QUaModbusRtuSerialClient::ComPorts = "QUaModbusRtuSerialClient::ComPorts";

QUaEnumMap QUaModbusRtuSerialClient::EnumComPorts()
{
	QUaEnumMap mapPorts;
	QList<QSerialPortInfo> list = QSerialPortInfo::availablePorts();
	for (int i = 0; i < list.count(); i++)
	{
		QSerialPortInfo portInfo = list.at(i);
		mapPorts.insert(i, 
			{ 
				{ "", portInfo.portName().toUtf8() },
				{ "", "" }
			}
		);
	}
	return mapPorts;
}

QDomElement QUaModbusRtuSerialClient::toDomElement(QDomDocument & domDoc) const
{
	// add client list element
	QDomElement elemSerialClient = domDoc.createElement(QUaModbusRtuSerialClient::staticMetaObject.className());
#ifdef QUA_ACCESS_CONTROL
	// set parmissions if any
	if (this->hasPermissionsObject())
	{
		elemSerialClient.setAttribute("Permissions", this->permissionsObject()->nodeId());
	}
#endif // QUA_ACCESS_CONTROL
	// set client attributes
	elemSerialClient.setAttribute("BrowseName"    , this->browseName()  );
	elemSerialClient.setAttribute("ServerAddress" , getServerAddress()  );
	elemSerialClient.setAttribute("KeepConnecting", getKeepConnecting() );
	elemSerialClient.setAttribute("ComPort"       , QString(QUaModbusRtuSerialClient::EnumComPorts().value(getComPortKey()).displayName.text()));
	elemSerialClient.setAttribute("Parity"        , QMetaEnum::fromType<QParity>  ().valueToKey(getParity()   ));
	elemSerialClient.setAttribute("BaudRate"      , QMetaEnum::fromType<QBaudRate>().valueToKey(getBaudRate() ));
	elemSerialClient.setAttribute("DataBits"      , QMetaEnum::fromType<QDataBits>().valueToKey(getDataBits() ));
	elemSerialClient.setAttribute("StopBits"      , QMetaEnum::fromType<QStopBits>().valueToKey(getStopBits() ));
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
	Q_ASSERT(this->browseName() == QUaQualifiedName(strBrowseName));
	bool bOK;
	// ServerAddress
	auto serverAddress = domElem.attribute("ServerAddress").toUInt(&bOK);
#ifdef QUA_ACCESS_CONTROL
	// load permissions if any
	if (domElem.hasAttribute("Permissions") && !domElem.attribute("Permissions").isEmpty())
	{
		strError += this->setPermissions(domElem.attribute("Permissions"));
	}
#endif // QUA_ACCESS_CONTROL
	if (bOK)
	{
		this->setServerAddress(serverAddress);
	}
	else
	{
		strError += tr("%1 : Invalid ServerAddress attribute '%2' in Modbus client %3. Default value set.\n").arg("Warning").arg(serverAddress).arg(strBrowseName);
	}
	// KeepConnecting
	auto keepConnecting = (bool)domElem.attribute("KeepConnecting").toUInt(&bOK);
	if (bOK)
	{
		this->setKeepConnecting(keepConnecting);
	}
	else
	{
		strError += tr("%1 : Invalid KeepConnecting attribute '%2' in Modbus client %3. Default value set.\n").arg("Warning").arg(keepConnecting).arg(strBrowseName);
	}
	// ComPort
	auto comPort = domElem.attribute("ComPort");
	if (!comPort.isEmpty())
	{
		this->setComPortKey(QUaModbusRtuSerialClient::EnumComPorts().key(
			{ 
				{ "", comPort.toUtf8() },
				{ "", "" }
			}, 0));
	}
	else
	{
		strError += tr("%1 : Invalid ComPort attribute '%2' in Modbus client %3. Default value set.\n").arg("Warning").arg(comPort).arg(strBrowseName);
	}
	// Parity
	auto strParity = domElem.attribute("Parity");
	auto parity = (QParity)QMetaEnum::fromType<QParity>().keysToValue(strParity.toUtf8(), &bOK);
	if (bOK)
	{
		this->setParity(parity);
	}
	else
	{
		strError += tr("%1 : Invalid Parity attribute '%2' in Modbus client %3. Default value set.\n").arg("Warning").arg(strParity).arg(strBrowseName);
	}
	// BaudRate
	auto strBaudRate = domElem.attribute("BaudRate");
	auto baudRate = (QBaudRate)QMetaEnum::fromType<QBaudRate>().keysToValue(strBaudRate.toUtf8(), &bOK);
	if (bOK)
	{
		this->setBaudRate(baudRate);
	}
	else
	{
		strError += tr("%1 : Invalid BaudRate attribute '%2' in Modbus client %3. Default value set.\n").arg("Warning").arg(strBaudRate).arg(strBrowseName);
	}
	// DataBits
	auto strDataBits = domElem.attribute("DataBits");
	auto dataBits = (QDataBits)QMetaEnum::fromType<QDataBits>().keysToValue(strDataBits.toUtf8(), &bOK);
	if (bOK)
	{
		this->setDataBits(dataBits);
	}
	else
	{
		strError += tr("%1 : Invalid DataBits attribute '%2' in Modbus client %3. Default value set.\n").arg("Warning").arg(strDataBits).arg(strBrowseName);
	}
	// StopBits
	auto strStopBits = domElem.attribute("StopBits");
	auto stopBits = (QStopBits)QMetaEnum::fromType<QStopBits>().keysToValue(strStopBits.toUtf8(), &bOK);
	if (bOK)
	{
		this->setStopBits(stopBits);
	}
	else
	{
		strError += tr("%1 : Invalid StopBits attribute '%2' in Modbus client %3. Default value set.\n").arg("Warning").arg(strStopBits).arg(strBrowseName);
	}
	// get block list
	QDomElement elemBlockList = domElem.firstChildElement(QUaModbusDataBlockList::staticMetaObject.className());
	if (!elemBlockList.isNull())
	{
		dataBlocks()->fromDomElement(elemBlockList, strError);
	}
	else
	{
		strError += tr("%1 : Modbus client %2 does not have a QUaModbusDataBlockList child. No blocks will be loaded.\n").arg("Warning").arg(strBrowseName);
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
	QString strComPort = QUaModbusRtuSerialClient::EnumComPorts().value(value.toInt()).displayName.text();
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
	return QUaModbusRtuSerialClient::EnumComPorts().value(key).displayName.text();
}

void QUaModbusRtuSerialClient::setComPort(const QString & strComPort)
{
	auto comPort = QUaModbusRtuSerialClient::EnumComPorts().key(
		{ 
			{ "", strComPort.toUtf8() },
			{ "", "" }
		}, 0);
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
