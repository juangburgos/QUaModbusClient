#include "quamodbustcpclient.h"

#ifdef QUA_ACCESS_CONTROL
#include <QUaPermissions>
#endif // QUA_ACCESS_CONTROL

QUaModbusTcpClient::QUaModbusTcpClient(QUaServer *server)
	: QUaModbusClient(server)
{
	// set defaults
	type          ()->setDataTypeEnum(QMetaEnum::fromType<QModbusClientType>());
	type          ()->setValue(QModbusClientType::Tcp);
	networkAddress()->setValue("127.0.0.1");
	networkPort   ()->setDataType(QMetaType::UShort);
	networkPort   ()->setValue(502);
	// set initial conditions
	networkAddress()->setWriteAccess(true);
	networkPort   ()->setWriteAccess(true);
	// instantiate client
	this->resetModbusClient();
	// handle changes
	QObject::connect(networkAddress(), &QUaBaseVariable::valueChanged, this, &QUaModbusTcpClient::on_networkAddressChanged, Qt::QueuedConnection);
	QObject::connect(networkPort()   , &QUaBaseVariable::valueChanged, this, &QUaModbusTcpClient::on_networkPortChanged   , Qt::QueuedConnection);
	// set descriptions
	/*
	networkAddress()->setDescription(tr("Network address (IP address or domain name) of the Modbus server."));
	networkPort()   ->setDescription(tr("Network port (TCP port) of the Modbus server."));
	*/
}

QUaProperty * QUaModbusTcpClient::networkAddress() const
{
	QMutexLocker locker(&(const_cast<QUaModbusTcpClient*>(this)->m_mutex));
	return const_cast<QUaModbusTcpClient*>(this)->browseChild<QUaProperty>("NetworkAddress");
}

QUaProperty * QUaModbusTcpClient::networkPort() const
{
	QMutexLocker locker(&(const_cast<QUaModbusTcpClient*>(this)->m_mutex));
	return const_cast<QUaModbusTcpClient*>(this)->browseChild<QUaProperty>("NetworkPort");
}

QString QUaModbusTcpClient::getNetworkAddress() const
{
	QMutexLocker locker(&(const_cast<QUaModbusTcpClient*>(this)->m_mutex));
	return this->networkAddress()->value().toString();
}

void QUaModbusTcpClient::setNetworkAddress(const QString & strNetworkAddress)
{
	QMutexLocker locker(&m_mutex);
	this->networkAddress()->setValue(strNetworkAddress);
	this->on_networkAddressChanged(strNetworkAddress);
}

quint16 QUaModbusTcpClient::getNetworkPort() const
{
	QMutexLocker locker(&(const_cast<QUaModbusTcpClient*>(this)->m_mutex));
	return this->networkPort()->value().value<quint16>();
}

void QUaModbusTcpClient::setNetworkPort(const quint16 & networkPort)
{
	QMutexLocker locker(&m_mutex);
	this->networkPort()->setValue(networkPort);
	this->on_networkPortChanged(networkPort);
}

void QUaModbusTcpClient::resetModbusClient()
{
	m_workerThread.execInThread([this]() {
		// instantiate in thread so it runs on the thread
		m_modbusClient.reset(new QModbusTcpClient(nullptr), [](QObject* client) {
			client->deleteLater();
		});
		// defaults
		m_modbusClient->setConnectionParameter(QModbusDevice::NetworkAddressParameter, this->getNetworkAddress());
		m_modbusClient->setConnectionParameter(QModbusDevice::NetworkPortParameter   , this->getNetworkPort   ());
		// setup client (call base class method)
		this->QUaModbusClient::resetModbusClient();
		QObject::connect(m_modbusClient.data(), &QModbusClient::stateChanged, this, &QUaModbusTcpClient::on_stateChanged, Qt::QueuedConnection);
	});
}

QDomElement QUaModbusTcpClient::toDomElement(QDomDocument & domDoc) const
{
	// add client element
	QDomElement elemTcpClient = domDoc.createElement(QUaModbusTcpClient::staticMetaObject.className());
#ifdef QUA_ACCESS_CONTROL
	// set parmissions if any
	if (this->hasPermissionsObject())
	{
		elemTcpClient.setAttribute("Permissions", this->permissionsObject()->nodeId());
	}
#endif // QUA_ACCESS_CONTROL
	// set client attributes
	elemTcpClient.setAttribute("BrowseName"    , this->browseName() );
	elemTcpClient.setAttribute("ServerAddress" , getServerAddress ());
	elemTcpClient.setAttribute("KeepConnecting", getKeepConnecting());
	elemTcpClient.setAttribute("NetworkAddress", getNetworkAddress());
	elemTcpClient.setAttribute("NetworkPort"   , getNetworkPort   ());
	// add block list element
	auto elemBlockList = dataBlocks()->toDomElement(domDoc);
	elemTcpClient.appendChild(elemBlockList);
	// return client element
	return elemTcpClient;
}

void QUaModbusTcpClient::fromDomElement(QDomElement & domElem, QQueue<QUaLog>& errorLogs)
{
	// get client attributes (BrowseName must be already set)
	QString strBrowseName = domElem.attribute("BrowseName");
	Q_ASSERT(this->browseName() == QUaQualifiedName(strBrowseName));
#ifdef QUA_ACCESS_CONTROL
	// load permissions if any
	if (domElem.hasAttribute("Permissions") && !domElem.attribute("Permissions").isEmpty())
	{
		QString strError = this->setPermissions(domElem.attribute("Permissions"));
		if (strError.contains("Error"))
		{
			errorLogs << QUaLog(
				strError,
				QUaLogLevel::Error,
				QUaLogCategory::Serialization
			);
		}
	}
#endif // QUA_ACCESS_CONTROL
	bool bOK;
	// ServerAddress
	auto serverAddress = domElem.attribute("ServerAddress").toUInt(&bOK);
	if (bOK)
	{
		this->setServerAddress(serverAddress);
	}
	else
	{
		errorLogs << QUaLog(
			tr("Invalid ServerAddress attribute '%1' in Modbus client %2. Ignoring.").arg(serverAddress).arg(strBrowseName),
			QUaLogLevel::Warning,
			QUaLogCategory::Serialization
		);
	}
	// KeepConnecting
	auto keepConnecting = (bool)domElem.attribute("KeepConnecting").toUInt(&bOK);
	if (bOK)
	{
		this->setKeepConnecting(keepConnecting);
	}
	else
	{
		errorLogs << QUaLog(
			tr("Invalid KeepConnecting attribute '%2' in Modbus client %2. Ignoring.").arg(keepConnecting).arg(strBrowseName),
			QUaLogLevel::Warning,
			QUaLogCategory::Serialization
		);
	}
	// NetworkAddress
	auto networkAddress = domElem.attribute("NetworkAddress");
	if (!networkAddress.isEmpty())
	{
		this->setNetworkAddress(networkAddress);
	}
	else
	{
		errorLogs << QUaLog(
			tr("Invalid NetworkAddress attribute '%1' in Modbus client %2. Default value set.").arg(networkAddress).arg(strBrowseName),
			QUaLogLevel::Warning,
			QUaLogCategory::Serialization
		);
	}
	// NetworkPort
	auto networkPort = domElem.attribute("NetworkPort").toUInt(&bOK);
	if (bOK)
	{
		this->setNetworkPort(networkPort);
	}
	else
	{
		errorLogs << QUaLog(
			tr("Invalid NetworkPort attribute '%1' in Modbus client %2. Default value set.").arg(networkPort).arg(strBrowseName),
			QUaLogLevel::Warning,
			QUaLogCategory::Serialization
		);
	}
	// get block list
	QDomElement elemBlockList = domElem.firstChildElement(QUaModbusDataBlockList::staticMetaObject.className());
	if (!elemBlockList.isNull())
	{
		dataBlocks()->fromDomElement(elemBlockList, errorLogs);
	}
	else
	{
		errorLogs << QUaLog(
			tr("Modbus client %1 does not have a QUaModbusDataBlockList child. No blocks will be loaded.").arg(strBrowseName),
			QUaLogLevel::Warning,
			QUaLogCategory::Serialization
		);
	}
}

void QUaModbusTcpClient::on_stateChanged(const QModbusDevice::State &state)
{
	// only allow to write connection params if not connected
	if (state == QModbusDevice::State::UnconnectedState)
	{
		networkAddress()->setWriteAccess(true);
		networkPort   ()->setWriteAccess(true);
	}
	else
	{
		networkAddress()->setWriteAccess(false);
		networkPort   ()->setWriteAccess(false);
	}
}

void QUaModbusTcpClient::on_networkAddressChanged(const QVariant & value)
{
	//Q_ASSERT_X(this->getState() == QModbusDevice::State::UnconnectedState);
	// NOTE : if connected, will not change until reconnect
	QString strNetworkAddress = value.toString();
	// set in thread, for thread-safety
	m_workerThread.execInThread([this, strNetworkAddress]() {
		m_modbusClient->setConnectionParameter(QModbusDevice::NetworkAddressParameter, strNetworkAddress);
	});
	// emit
	emit this->networkAddressChanged(strNetworkAddress);
}

void QUaModbusTcpClient::on_networkPortChanged(const QVariant & value)
{
	//Q_ASSERT(this->getState() == QModbusDevice::State::UnconnectedState);
	// NOTE : if connected, will not change until reconnect
	quint16 uiPort = value.value<quint16>();
	// set in thread, for thread-safety
	m_workerThread.execInThread([this, uiPort]() {
		m_modbusClient->setConnectionParameter(QModbusDevice::NetworkPortParameter, uiPort);
	});
	// emit
	emit this->networkPortChanged(uiPort);
}