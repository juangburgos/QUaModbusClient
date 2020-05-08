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
	m_workerThread.execInThread([this]() {
		// instantiate in thread so it runs on the thread
		m_modbusClient.reset(new QModbusTcpClient(nullptr), [](QObject * client) {
			client->deleteLater();
		});
		// defaults
		m_modbusClient->setConnectionParameter(QModbusDevice::NetworkAddressParameter, "127.0.0.1");
		m_modbusClient->setConnectionParameter(QModbusDevice::NetworkPortParameter   , 502);
		// setup client (call base class method)
		this->setupModbusClient();
		QObject::connect(m_modbusClient.data(), &QModbusClient::stateChanged, this, &QUaModbusTcpClient::on_stateChanged, Qt::QueuedConnection);
	});
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
	return const_cast<QUaModbusTcpClient*>(this)->browseChild<QUaProperty>("NetworkAddress");
}

QUaProperty * QUaModbusTcpClient::networkPort() const
{
	return const_cast<QUaModbusTcpClient*>(this)->browseChild<QUaProperty>("NetworkPort");
}

QString QUaModbusTcpClient::getNetworkAddress() const
{
	return this->networkAddress()->value().toString();
}

void QUaModbusTcpClient::setNetworkAddress(const QString & strNetworkAddress)
{
	this->networkAddress()->setValue(strNetworkAddress);
	this->on_networkAddressChanged(strNetworkAddress);
}

quint16 QUaModbusTcpClient::getNetworkPort() const
{
	return this->networkPort()->value().value<quint16>();
}

void QUaModbusTcpClient::setNetworkPort(const quint16 & networkPort)
{
	this->networkPort()->setValue(networkPort);
	this->on_networkPortChanged(networkPort);
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

void QUaModbusTcpClient::fromDomElement(QDomElement & domElem, QString & strError)
{
	// get client attributes (BrowseName must be already set)
	QString strBrowseName = domElem.attribute("BrowseName");
	Q_ASSERT(this->browseName() == QUaQualifiedName(strBrowseName));
#ifdef QUA_ACCESS_CONTROL
	// load permissions if any
	if (domElem.hasAttribute("Permissions") && !domElem.attribute("Permissions").isEmpty())
	{
		strError += this->setPermissions(domElem.attribute("Permissions"));
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
		strError += tr("%1 : Invalid ServerAddress attribute '%2' in Modbus client %3. Ignoring.\n").arg("Warning").arg(serverAddress).arg(strBrowseName);
	}
	// KeepConnecting
	auto keepConnecting = (bool)domElem.attribute("KeepConnecting").toUInt(&bOK);
	if (bOK)
	{
		this->setKeepConnecting(keepConnecting);
	}
	else
	{
		strError += tr("%1 : Invalid KeepConnecting attribute '%2' in Modbus client %3. Ignoring.\n").arg("Warning").arg(keepConnecting).arg(strBrowseName);
	}
	// NetworkAddress
	auto networkAddress = domElem.attribute("NetworkAddress");
	if (!networkAddress.isEmpty())
	{
		this->setNetworkAddress(networkAddress);
	}
	else
	{
		strError += tr("%1 : Invalid NetworkAddress attribute '%2' in Modbus client %3. Default value set.\n").arg("Warning").arg(networkAddress).arg(strBrowseName);
	}
	// NetworkPort
	auto networkPort = domElem.attribute("NetworkPort").toUInt(&bOK);
	if (bOK)
	{
		this->setNetworkPort(networkPort);
	}
	else
	{
		strError += tr("%1 : Invalid NetworkPort attribute '%2' in Modbus client %3. Default value set.\n").arg("Warning").arg(networkPort).arg(strBrowseName);
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