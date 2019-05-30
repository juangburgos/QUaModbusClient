#include "quamodbustcpclient.h"

QUaModbusTcpClient::QUaModbusTcpClient(QUaServer *server)
	: QUaModbusClient(server)
{
	// set defaults
	type          ()->setValue("Tcp");
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
	networkAddress()->setDescription(tr("Network address (IP address or domain name) of the Modbus server."));
	networkPort()   ->setDescription(tr("Network port (TCP port) of the Modbus server."));
}

QUaProperty * QUaModbusTcpClient::networkAddress() const
{
	return this->browseChild<QUaProperty>("NetworkAddress");
}

QUaProperty * QUaModbusTcpClient::networkPort() const
{
	return this->browseChild<QUaProperty>("NetworkPort");
}

QDomElement QUaModbusTcpClient::toDomElement(QDomDocument & domDoc) const
{
	// add client element
	QDomElement elemTcpClient = domDoc.createElement(QUaModbusTcpClient::staticMetaObject.className());
	// set client attributes
	elemTcpClient.setAttribute("BrowseName"    , this->browseName());
	elemTcpClient.setAttribute("ServerAddress" , serverAddress ()->value().toUInt());
	elemTcpClient.setAttribute("KeepConnecting", keepConnecting()->value().toBool());
	elemTcpClient.setAttribute("NetworkAddress", networkAddress()->value().toString());
	elemTcpClient.setAttribute("NetworkPort"   , networkPort   ()->value().toUInt());
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
		strError += QString(tr("Error : Invalid ServerAddress attribute '%1' in Modbus client %2. Ignoring.\n")).arg(serverAddress).arg(strBrowseName);
	}
	// KeepConnecting
	auto keepConnecting = (bool)domElem.attribute("KeepConnecting").toUInt(&bOK);
	if (bOK)
	{
		this->keepConnecting()->setValue(keepConnecting);
	}
	else
	{
		strError += QString(tr("Error : Invalid KeepConnecting attribute '%1' in Modbus client %2. Ignoring.\n")).arg(keepConnecting).arg(strBrowseName);
	}
	// NetworkAddress
	auto networkAddress = domElem.attribute("NetworkAddress");
	if (!networkAddress.isEmpty())
	{
		this->networkAddress()->setValue(networkAddress);
		// NOTE : force internal update (if connected won't apply until reconnect)
		this->on_networkAddressChanged(networkAddress);
	}
	else
	{
		strError += QString(tr("Error : Invalid NetworkAddress attribute '%1' in Modbus client %2. Ignoring.\n")).arg(networkAddress).arg(strBrowseName);
	}
	// NetworkPort
	auto networkPort = domElem.attribute("NetworkPort").toUInt(&bOK);
	if (bOK)
	{
		this->networkPort()->setValue(networkPort);
		// NOTE : force internal update (if connected won't apply until reconnect)
		this->on_networkPortChanged(networkPort);
	}
	else
	{
		strError += QString(tr("Error : Invalid NetworkPort attribute '%1' in Modbus client %2. Ignoring.\n")).arg(networkPort).arg(strBrowseName);
	}
	// get block list
	QDomElement elemBlockList = domElem.firstChildElement(QUaModbusDataBlockList::staticMetaObject.className());
	if (!elemBlockList.isNull())
	{
		dataBlocks()->fromDomElement(elemBlockList, strError);
	}
	else
	{
		strError += QString(tr("Error : Modbus client %1 does not have a QUaModbusDataBlockList child. No blocks will be loaded.\n")).arg(strBrowseName);
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

}