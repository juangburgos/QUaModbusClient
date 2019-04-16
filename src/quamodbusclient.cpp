#include "quamodbusclient.h"

QUaModbusClient::QUaModbusClient(QUaServer *server)
	: QUaBaseObject(server)
{
	// set defaults
	state    ()->setDataTypeEnum(QMetaEnum::fromType<QModbusDevice::State>());
	state    ()->setValue(QModbusDevice::State::UnconnectedState);
	lastError()->setDataTypeEnum(QMetaEnum::fromType<QModbusDevice::Error>());
	lastError()->setValue(QModbusDevice::Error::NoError);
}

QUaProperty * QUaModbusClient::type()
{
	return this->browseChild<QUaProperty>("Type");
}

QUaBaseDataVariable * QUaModbusClient::state()
{
	return this->browseChild<QUaBaseDataVariable>("State");
}

QUaBaseDataVariable * QUaModbusClient::lastError()
{
	return this->browseChild<QUaBaseDataVariable>("LastError");
}

QUaModbusDataBlockList * QUaModbusClient::dataBlocks()
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

void QUaModbusClient::on_stateChanged(QModbusDevice::State state)
{
	this->state()->setValue(state);
	// no error if connected correctly
	if (state == QModbusDevice::State::ConnectedState)
	{
		this->lastError()->setValue(QModbusDevice::Error::NoError);
	}
}

void QUaModbusClient::on_errorChanged(QModbusDevice::Error error)
{
	this->lastError()->setValue(error);
	// TODO : trigger UA Event
}