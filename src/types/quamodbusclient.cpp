#include "quamodbusclient.h"

#include <QUaModbusDataBlock>

QUaModbusClient::QUaModbusClient(QUaServer *server)
	: QUaBaseObject(server)
{
	if (QMetaType::type("QModbusError") == QMetaType::UnknownType)
	{
		qRegisterMetaType<QModbusError>("QModbusError");
	}
	if (QMetaType::type("QModbusState") == QMetaType::UnknownType)
	{
		qRegisterMetaType<QModbusState>("QModbusState");
	}
	// set defaults
	state         ()->setDataTypeEnum(QMetaEnum::fromType<QModbusState>());
	state         ()->setValue(QModbusState::UnconnectedState);
	lastError     ()->setDataTypeEnum(QMetaEnum::fromType<QModbusError>());
	lastError     ()->setValue(QModbusError::NoError);
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
	// handle changes
	QObject::connect(serverAddress() , &QUaBaseVariable::valueChanged, this, &QUaModbusClient::on_serverAddressChanged , Qt::QueuedConnection);
	QObject::connect(keepConnecting(), &QUaBaseVariable::valueChanged, this, &QUaModbusClient::on_keepConnectingChanged, Qt::QueuedConnection);
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

quint8 QUaModbusClient::getServerAddress() const
{
	return this->serverAddress()->value().value<quint8>();
}

void QUaModbusClient::setServerAddress(const quint8 & serverAddress)
{
	this->serverAddress()->setValue(serverAddress);
	this->on_serverAddressChanged(serverAddress);
}

bool QUaModbusClient::getKeepConnecting() const
{
	return this->keepConnecting()->value().toBool();
}

void QUaModbusClient::setKeepConnecting(const bool & keepConnecting)
{
	this->keepConnecting()->setValue(keepConnecting);
	this->on_keepConnectingChanged(keepConnecting);
}

QModbusError QUaModbusClient::getLastError() const
{
	return this->lastError()->value().value<QModbusError>();
}

void QUaModbusClient::setLastError(const QModbusError & error)
{
	this->lastError()->setValue(error);
	this->on_errorChanged(error);
}

QModbusClientType QUaModbusClient::getType() const
{
	return this->type()->value().value<QModbusClientType>();
}

QModbusState QUaModbusClient::getState() const
{
	return this->state()->value().value<QModbusState>();
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

void QUaModbusClient::on_serverAddressChanged(const QVariant & value)
{
	// emit
	emit this->serverAddressChanged(value.value<quint8>());
}

void QUaModbusClient::on_keepConnectingChanged(const QVariant & value)
{
	// emit
	emit this->keepConnectingChanged(value.toBool());
}

void QUaModbusClient::on_stateChanged(QModbusState state)
{
	this->state()->setValue(state);
	// no error if connected correctly
	if (state == QModbusState::ConnectedState)
	{
		this->lastError()->setValue(QModbusError::NoError);
	}
	// only allow to write connection params if not connected
	if (state == QModbusState::UnconnectedState)
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
	//        only works for changes through network
	// emit
	emit this->stateChanged(state);
	// update block errors
	if (state == QModbusState::ConnectedState)
	{
		return;
	}
	auto blocks = this->dataBlocks()->blocks();
	for (int i = 0; i < blocks.count(); i++)
	{
		blocks.at(i)->setLastError(QModbusError::ConnectionError);
	}
}

void QUaModbusClient::on_errorChanged(QModbusError error)
{
	this->lastError()->setValue(error);
	if (error != QModbusError::NoError)
	{
		// TODO : send UA event
	}
	// NOTE : need to add custom signal because OPC UA valueChanged
	//        only works for changes through network
	// emit
	emit this->lastErrorChanged(error);
}