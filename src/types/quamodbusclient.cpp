#include "quamodbusclient.h"

#include <QMutexLocker>

#include <QUaModbusDataBlock>
#include <QUaModbusClientList>

QUaModbusClient::QUaModbusClient(QUaServer *server)
#ifndef QUA_ACCESS_CONTROL
	: QUaBaseObject(server)
#else
	: QUaBaseObjectProtected(server)
#endif // !QUA_ACCESS_CONTROL
	, m_mutex(QMutex::Recursive)
{
	m_disconnectRequested = false;
	m_type = nullptr;
	m_serverAddress = nullptr;
	m_keepConnecting = nullptr;
	m_state = nullptr;
	m_lastError = nullptr;
	m_dataBlocks = nullptr;
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
	/*
	type          ()->setDescription(tr("Modbus client communication type (TCP or RTU Serial)."));
	serverAddress ()->setDescription(tr("Modbus server Device Id or Modbus address."));
	keepConnecting()->setDescription(tr("Whether the client should try to keep connecting after connection failure"));
	state         ()->setDescription(tr("Modbus connection state."));
	lastError     ()->setDescription(tr("Last error occured at connection level."));
	dataBlocks    ()->setDescription(tr("List of Modbus data blocks updated through polling."));
	*/
	// handle changes
	QObject::connect(serverAddress() , &QUaBaseVariable::valueChanged, this, &QUaModbusClient::on_serverAddressChanged , Qt::QueuedConnection);
	QObject::connect(keepConnecting(), &QUaBaseVariable::valueChanged, this, &QUaModbusClient::on_keepConnectingChanged, Qt::QueuedConnection);
}

QUaModbusClient::~QUaModbusClient()
{
	emit this->aboutToDestroy();
	emit m_dataBlocks->aboutToClear();
	// delete while client still valid, because in views blocks reference parent client
	for (auto block : m_dataBlocks->blocks())
	{
		delete block;
	}
}

QUaProperty * QUaModbusClient::type()
{
	QMutexLocker locker(&this->m_mutex);
	if (!m_type)
	{
		m_type = this->browseChild<QUaProperty>("Type");
	}
	return m_type;
}

QUaProperty * QUaModbusClient::serverAddress()
{
	QMutexLocker locker(&this->m_mutex);
	if (!m_serverAddress)
	{
		m_serverAddress = this->browseChild<QUaProperty>("ServerAddress");
	}
	return m_serverAddress;
}

QUaProperty * QUaModbusClient::keepConnecting()
{
	QMutexLocker locker(&this->m_mutex);
	if (!m_keepConnecting)
	{
		m_keepConnecting = this->browseChild<QUaProperty>("KeepConnecting");
	}
	return m_keepConnecting;
}

QUaBaseDataVariable * QUaModbusClient::state()
{
	QMutexLocker locker(&this->m_mutex);
	if (!m_state)
	{
		m_state = this->browseChild<QUaBaseDataVariable>("State");
	}
	return m_state;
}

QUaBaseDataVariable * QUaModbusClient::lastError()
{
	QMutexLocker locker(&this->m_mutex);
	if (!m_lastError)
	{
		m_lastError = this->browseChild<QUaBaseDataVariable>("LastError");
	}
	return m_lastError;
}

QUaModbusDataBlockList * QUaModbusClient::dataBlocks()
{
	QMutexLocker locker(&this->m_mutex);
	if (!m_dataBlocks)
	{
		m_dataBlocks = this->browseChild<QUaModbusDataBlockList>("DataBlocks");
	}
	return m_dataBlocks;
}

void QUaModbusClient::remove()
{
	QMutexLocker locker(&m_mutex);
	this->disconnectDevice();
	this->deleteLater();
}

void QUaModbusClient::connectDevice()
{
	QMutexLocker locker(&m_mutex);
	// check if same
	if (this->getState() == QModbusState::ConnectedState)
	{
		return;
	}
	// exec in thread, for thread-safety
	m_workerThread.execInThread([this]() {
		m_modbusClient->connectDevice();
	});
}

void QUaModbusClient::disconnectDevice()
{
	QMutexLocker locker(&m_mutex);
	// check if same
	if (this->getState() == QModbusState::UnconnectedState)
	{
		return;
	}
	// exec in thread, for thread-safety
	m_workerThread.execInThread([this]() {
		// NOTE : reset pointer in order to reduce "ClosingState" large timeouts 
		//        for requested disconnections on unexisting servers
		m_disconnectRequested = true;
		emit m_modbusClient.data()->stateChanged(QModbusState::UnconnectedState);
		QObject::disconnect(m_modbusClient.data());
		QObject::disconnect(m_modbusClient.data(), &QModbusClient::stateChanged , this, &QUaModbusClient::on_stateChanged);
		QObject::disconnect(m_modbusClient.data(), &QModbusClient::errorOccurred, this, &QUaModbusClient::on_errorChanged);			
		m_modbusClient->disconnectDevice();	
		this->resetModbusClient();
	});
}

quint8 QUaModbusClient::getServerAddress() const
{
	QMutexLocker locker(&(const_cast<QUaModbusClient*>(this)->m_mutex));
	return const_cast<QUaModbusClient*>(this)->serverAddress()->value().value<quint8>();
}

void QUaModbusClient::setServerAddress(const quint8 & serverAddress)
{
	QMutexLocker locker(&m_mutex);
	this->serverAddress()->setValue(serverAddress);
	this->on_serverAddressChanged(serverAddress, true);
}

bool QUaModbusClient::getKeepConnecting() const
{
	QMutexLocker locker(&(const_cast<QUaModbusClient*>(this)->m_mutex));
	return const_cast<QUaModbusClient*>(this)->keepConnecting()->value().toBool();
}

void QUaModbusClient::setKeepConnecting(const bool & keepConnecting)
{
	QMutexLocker locker(&m_mutex);
	this->keepConnecting()->setValue(keepConnecting);
	this->on_keepConnectingChanged(keepConnecting, true);
}

QModbusError QUaModbusClient::getLastError() const
{
	QMutexLocker locker(&(const_cast<QUaModbusClient*>(this)->m_mutex));
	return const_cast<QUaModbusClient*>(this)->lastError()->value().value<QModbusError>();
}

void QUaModbusClient::setLastError(const QModbusError & error)
{
	QMutexLocker locker(&m_mutex);
	this->on_errorChanged(error);
}

QUaModbusClientList * QUaModbusClient::list() const
{
	QMutexLocker locker(&(const_cast<QUaModbusClient*>(this)->m_mutex));
	return qobject_cast<QUaModbusClientList*>(this->parent());
}

QModbusClientType QUaModbusClient::getType() const
{
	QMutexLocker locker(&(const_cast<QUaModbusClient*>(this)->m_mutex));
	return const_cast<QUaModbusClient*>(this)->type()->value().value<QModbusClientType>();
}

QModbusState QUaModbusClient::getState() const
{
	QMutexLocker locker(&(const_cast<QUaModbusClient*>(this)->m_mutex));
	return const_cast<QUaModbusClient*>(this)->state()->value().value<QModbusState>();
}

void QUaModbusClient::setState(const QModbusState & state)
{
	QMutexLocker locker(&m_mutex);
	this->state()->setValue(state);
	// NOTE : need to add custom signal because OPC UA valueChanged
	//        only works for changes through network
	// emit
	emit this->stateChanged(state);
}

void QUaModbusClient::resetModbusClient()
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

void QUaModbusClient::fromDomElement(QDomElement & domElem, QQueue<QUaLog>& errorLogs)
{
	// must never reach here
	Q_ASSERT(false);
	Q_UNUSED(domElem);
	Q_UNUSED(errorLogs);
}

void QUaModbusClient::on_serverAddressChanged(const QVariant & value, const bool& networkChange)
{

	if (!networkChange)
	{
		return;
	}
	// emit
	emit this->serverAddressChanged(value.value<quint8>());
}

void QUaModbusClient::on_keepConnectingChanged(const QVariant & value, const bool& networkChange)
{

	if (!networkChange)
	{
		return;
	}
	// emit
	emit this->keepConnectingChanged(value.toBool());
}

void QUaModbusClient::on_stateChanged(QModbusState state)
{
	this->setState(state);
	// no error if connected correctly
	if (state == QModbusState::ConnectedState)
	{
		this->setLastError(QModbusError::NoError);
	}
	// only allow to write connection params if not connected
	if (state == QModbusState::UnconnectedState)
	{
		this->serverAddress()->setWriteAccess(true);
		// keep connecting if desired
		bool keepConnecting = this->keepConnecting()->value().toBool();
		if (keepConnecting && !m_disconnectRequested)
		{
			this->connectDevice();
		}
		m_disconnectRequested = false;
	}
	else
	{
		serverAddress()->setWriteAccess(false);
	}
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

// NOTE : need to add custom signal because OPC UA valueChanged
//        only works for changes through network
void QUaModbusClient::on_errorChanged(QModbusError error)
{
	// NOTe : setLastError call this, avoid recursion
	this->lastError()->setValue(error);
	//// TODO : send UA event
	//if (error != QModbusError::NoError)
	// emit
	emit this->lastErrorChanged(error);
}