#include "quamodbusclient.h"

QUaModbusClient::QUaModbusClient(QUaServer *server)
	: QUaBaseObject(server)
{
	// set defaults
	state         ()->setDataTypeEnum(QMetaEnum::fromType<QModbusDevice::State>());
	state         ()->setValue(QModbusDevice::State::UnconnectedState);
	lastError     ()->setDataTypeEnum(QMetaEnum::fromType<QModbusDevice::Error>());
	lastError     ()->setValue(QModbusDevice::Error::NoError);
	serverAddress ()->setDataType(QMetaType::UChar);
	serverAddress ()->setValue(1);
	keepConnecting()->setValue(false);
	// set initial conditions
	serverAddress ()->setWriteAccess(true);
	keepConnecting()->setWriteAccess(true);
	// set descriptions
	type          ()->setDescription(tr("Modbus client communication type (TCP or RTU Serial)."));
	serverAddress ()->setDescription(tr("Modbus server Device Id or Modbus address."));
	keepConnecting()->setDescription(tr("Whether the client should try to keep connecting after connection failure"));
	state         ()->setDescription(tr("Modbus connection state."));
	lastError     ()->setDescription(tr("Last error occured at connection level."));
	dataBlocks    ()->setDescription(tr("List of Modbus data blocks updated through polling."));
}

QUaProperty * QUaModbusClient::type() const
{
	return this->browseChild<QUaProperty>("Type");
}

QUaProperty * QUaModbusClient::serverAddress() const
{
	return this->browseChild<QUaProperty>("ServerAddress");
}

QUaProperty * QUaModbusClient::keepConnecting() const
{
	return this->browseChild<QUaProperty>("KeepConnecting");
}

QUaBaseDataVariable * QUaModbusClient::state() const
{
	return this->browseChild<QUaBaseDataVariable>("State");
}

QUaBaseDataVariable * QUaModbusClient::lastError() const
{
	return this->browseChild<QUaBaseDataVariable>("LastError");
}

QUaModbusDataBlockList * QUaModbusClient::dataBlocks() const
{
	return this->browseChild<QUaModbusDataBlockList>("DataBlocks");
}

void QUaModbusClient::remove()
{
	this->disconnectDevice();
	this->deleteLater();
}

void QUaModbusClient::connectDevice()
{
	// exec in thread, for thread-safety
	m_workerThread.execInThread([this]() {
		m_modbusClient->connectDevice();
	});
}

void QUaModbusClient::disconnectDevice()
{
	// exec in thread, for thread-safety
	m_workerThread.execInThread([this]() {
		m_modbusClient->disconnectDevice();
	});
}

QModbusDevice::State QUaModbusClient::getState()
{
	return this->state()->value().value<QModbusDevice::State>();
}

void QUaModbusClient::setupModbusClient()
{
	// subscribe to events
	QObject::connect(m_modbusClient.data(), &QModbusClient::stateChanged , this, &QUaModbusClient::on_stateChanged, Qt::QueuedConnection);
	QObject::connect(m_modbusClient.data(), &QModbusClient::errorOccurred, this, &QUaModbusClient::on_errorChanged, Qt::QueuedConnection);
}

QDomElement QUaModbusClient::toDomElement(QDomDocument & domDoc) const
{
	// must never reach here
	Q_ASSERT(false);
	Q_UNUSED(domDoc);
	return QDomElement();
}

void QUaModbusClient::fromDomElement(QDomElement & domElem, QString & strError)
{
	// must never reach here
	Q_ASSERT(false);
	Q_UNUSED(domElem);
	Q_UNUSED(strError);
}

void QUaModbusClient::on_stateChanged(QModbusDevice::State state)
{
	this->state()->setValue(state);
	// no error if connected correctly
	if (state == QModbusDevice::State::ConnectedState)
	{
		this->lastError()->setValue(QModbusDevice::Error::NoError);
	}
	// only allow to write connection params if not connected
	if (state == QModbusDevice::State::UnconnectedState)
	{
		serverAddress()->setWriteAccess(true);
		// keep connecting if desired
		bool keepConnecting = this->keepConnecting()->value().toBool();
		if (keepConnecting)
		{
			this->connectDevice();
		}
	}
	else
	{
		serverAddress()->setWriteAccess(false);
	}
	// NOTE : need to add custom signal because OPC UA valueChanged
	//        only works for changed through network
	// emit
	emit this->stateChanged(state);
}

void QUaModbusClient::on_errorChanged(QModbusDevice::Error error)
{
	this->lastError()->setValue(error);
	if (error != QModbusDevice::Error::NoError)
	{
		// TODO : send UA event
	}
}