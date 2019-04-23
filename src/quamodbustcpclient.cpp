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
		m_modbusClient.reset(new QModbusTcpClient(nullptr));
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
	networkAddress()->setDescription("Network address (IP address or domain name) of the Modbus server.");
	networkPort()   ->setDescription("Network port (TCP port) of the Modbus server.");
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
	QDomElement elemTcpClient = domDoc.createElement(QUaModbusTcpClient::metaObject()->className());
	// set client attributes
	elemTcpClient.setAttribute("BrowseName"    , this->browseName());
	elemTcpClient.setAttribute("ServerAddress" , serverAddress()->value().toUInt());
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
	// TODO
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
	Q_ASSERT_X(this->getState() == QModbusDevice::State::UnconnectedState,
		"QUaModbusTcpClient::on_networkAddressChanged", 
		"Cannot change network address while connected.");
	QString strNetworkAddress = value.toString();
	// set in thread, for thread-safety
	m_workerThread.execInThread([this, strNetworkAddress]() {
		m_modbusClient->setConnectionParameter(QModbusDevice::NetworkAddressParameter, strNetworkAddress);
	});
}

void QUaModbusTcpClient::on_networkPortChanged(const QVariant & value)
{
	Q_ASSERT_X(this->getState() == QModbusDevice::State::UnconnectedState,
		"QUaModbusTcpClient::on_networkPortChanged",
		"Cannot change network port while connected.");
	quint16 uiPort = value.value<quint16>();
	// set in thread, for thread-safety
	m_workerThread.execInThread([this, uiPort]() {
		m_modbusClient->setConnectionParameter(QModbusDevice::NetworkPortParameter, uiPort);
	});

}