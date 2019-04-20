#include "quamodbusdatablock.h"
#include "quamodbusclient.h"
#include "quamodbusvalue.h"

quint32 QUaModbusDataBlock::m_minSamplingTime = 50;

QUaModbusDataBlock::QUaModbusDataBlock(QUaServer *server)
	: QUaBaseObject(server)
{
	m_replyRead  = nullptr;
	// NOTE : QObject parent might not be yet available in constructor
	type   ()->setDataTypeEnum(QMetaEnum::fromType<QUaModbusDataBlock::RegisterType>());
	type   ()->setValue(QUaModbusDataBlock::RegisterType::Invalid);
	address()->setDataType(QMetaType::Int);
	address()->setValue(-1);
	size   ()->setDataType(QMetaType::UInt);
	size   ()->setValue(0);
	samplingTime()->setDataType(QMetaType::UInt);
	samplingTime()->setValue(1000);
	lastError   ()->setDataTypeEnum(QMetaEnum::fromType<QModbusDevice::Error>());
	lastError   ()->setValue(QModbusDevice::Error::NoError);
	// set initial conditions
	type()        ->setWriteAccess(true);
	address()     ->setWriteAccess(true);
	size()        ->setWriteAccess(true);
	samplingTime()->setWriteAccess(true);
	data()        ->setMinimumSamplingInterval(1000);
	// handle state changes
	QObject::connect(type()        , &QUaBaseVariable::valueChanged, this, &QUaModbusDataBlock::on_typeChanged        , Qt::QueuedConnection);
	QObject::connect(address()     , &QUaBaseVariable::valueChanged, this, &QUaModbusDataBlock::on_addressChanged     , Qt::QueuedConnection);
	QObject::connect(size()        , &QUaBaseVariable::valueChanged, this, &QUaModbusDataBlock::on_sizeChanged        , Qt::QueuedConnection);
	QObject::connect(samplingTime(), &QUaBaseVariable::valueChanged, this, &QUaModbusDataBlock::on_samplingTimeChanged, Qt::QueuedConnection);
	QObject::connect(data()        , &QUaBaseVariable::valueChanged, this, &QUaModbusDataBlock::on_dataChanged        , Qt::QueuedConnection);
}

QUaProperty * QUaModbusDataBlock::type()
{
	return this->browseChild<QUaProperty>("Type");
}

QUaProperty * QUaModbusDataBlock::address()
{
	return this->browseChild<QUaProperty>("Address");
}

QUaProperty * QUaModbusDataBlock::size()
{
	return this->browseChild<QUaProperty>("Size");
}

QUaProperty * QUaModbusDataBlock::samplingTime()
{
	return this->browseChild<QUaProperty>("SamplingTime");
}

QUaBaseDataVariable * QUaModbusDataBlock::data()
{
	return this->browseChild<QUaBaseDataVariable>("Data");
}

QUaBaseDataVariable * QUaModbusDataBlock::lastError()
{
	return this->browseChild<QUaBaseDataVariable>("LastError");
}

QUaModbusValueList * QUaModbusDataBlock::values()
{
	return this->browseChild<QUaModbusValueList>("Values");
}

void QUaModbusDataBlock::remove()
{
	this->deleteLater();
}

void QUaModbusDataBlock::on_typeChanged(const QVariant &value)
{
	auto type = value.value<QModbusDataUnit::RegisterType>();
	// set in thread for safety
	this->client()->m_workerThread.execInThread([this, type]() {
		m_modbusDataUnit.setRegisterType(type);
	});
	// set data writable according to type
	if (type == QUaModbusDataBlock::RegisterType::Coils ||
		type == QUaModbusDataBlock::RegisterType::HoldingRegisters)
	{
		data()->setWriteAccess(true);
	}
	else
	{
		data()->setWriteAccess(false);
	}
}

void QUaModbusDataBlock::on_addressChanged(const QVariant & value)
{
	auto address = value.value<int>();
	// set in thread for safety
	this->client()->m_workerThread.execInThread([this, address]() {
		m_modbusDataUnit.setStartAddress(address);
	});
}

void QUaModbusDataBlock::on_sizeChanged(const QVariant & value)
{
	auto size = value.value<quint32>();
	// set in thread for safety
	this->client()->m_workerThread.execInThread([this, size]() {
		m_modbusDataUnit.setValueCount(size);
	});
}

void QUaModbusDataBlock::on_samplingTimeChanged(const QVariant & value)
{
	// check minimum sampling time
	auto samplingTime = value.value<quint32>();
	// do not allow less than minimum
	if (samplingTime < QUaModbusDataBlock::m_minSamplingTime)
	{
		// set minumum
		this->samplingTime()->setValue(QUaModbusDataBlock::m_minSamplingTime);
		// the previous will trigger the event again
		return;
	}
	// stop old loop
	this->client()->m_workerThread.stopLoopInThread(m_loopHandle);
	// start new loop
	this->startLoop();
	// update ua sample interval for data
	this->data()->setMinimumSamplingInterval((double)samplingTime);
}

void QUaModbusDataBlock::on_dataChanged(const QVariant & value)
{
	// should not happen but just in case
	auto type = this->type()->value().value<QModbusDataUnit::RegisterType>();
	if (type != QModbusDataUnit::RegisterType::Coils &&
		type != QModbusDataUnit::RegisterType::HoldingRegisters)
	{
		return;
	}
	// convert data
	QVector<quint16> data;
	QSequentialIterable iterable = value.value<QSequentialIterable>();
	QSequentialIterable::const_iterator it = iterable.begin();
	const QSequentialIterable::const_iterator end = iterable.end();
	for (; it != end; ++it) {
		data << (*it).value<quint16>();
	}
	// exec write request in client thread
	this->client()->m_workerThread.execInThread([this, data]() {
		auto client = this->client();
		// check if connected
		auto state = client->state()->value().value<QModbusDevice::State>();
		if (state != QModbusDevice::State::ConnectedState)
		{
			return;
		}
		// check if request is valid
		if (m_modbusDataUnit.registerType() != QModbusDataUnit::RegisterType::Coils &&
			m_modbusDataUnit.registerType() != QModbusDataUnit::RegisterType::HoldingRegisters)
		{
			return;
		}
		if (m_modbusDataUnit.startAddress() < 0)
		{
			return;
		}
		if (m_modbusDataUnit.valueCount() == 0)
		{
			return;
		}
		// create data target 
		QModbusDataUnit dataToWrite(m_modbusDataUnit.registerType(), m_modbusDataUnit.startAddress(), data);
		// create and send request
		auto serverAddress = client->serverAddress()->value().value<quint8>();
		QModbusReply * p_reply = client->m_modbusClient->sendWriteRequest(dataToWrite, serverAddress);
		if (!p_reply)
		{
			// TODO : send UA event
			return;
		}
		// subscribe to finished
		QObject::connect(p_reply, &QModbusReply::finished, this, [this, p_reply]() mutable {
			// check if reply still valid
			if (!p_reply)
			{
				return;
			}
			// handle error
			auto error = p_reply->error();
			this->lastError()->setValue(error);
			if (error != QModbusDevice::Error::NoError)
			{
				// TODO : send UA event
			}
			// delete reply on next event loop exec
			p_reply->deleteLater();
			p_reply = nullptr;
		}, Qt::QueuedConnection);
	});
}

QUaModbusClient * QUaModbusDataBlock::client()
{
	return dynamic_cast<QUaModbusDataBlockList*>(this->parent())->client();
}

void QUaModbusDataBlock::startLoop()
{
	auto samplingTime = this->samplingTime()->value().value<quint32>();
	// exec read request in client thread
	m_loopHandle = this->client()->m_workerThread.startLoopInThread([this]() {
		auto client = this->client();
		// check if ongoing request
		if (m_replyRead)
		{
			return;
		}
		// check if connected
		auto state = client->state()->value().value<QModbusDevice::State>();
		if (state != QModbusDevice::State::ConnectedState)
		{
			return;
		}
		// check if request is valid
		if (m_modbusDataUnit.registerType() == QModbusDataUnit::RegisterType::Invalid)
		{
			return;
		}
		if (m_modbusDataUnit.startAddress() < 0)
		{
			return;
		}
		if (m_modbusDataUnit.valueCount() == 0)
		{
			return;
		}
		// create and send request		
		auto serverAddress = client->serverAddress()->value().value<quint8>();
		// NOTE : need to pass in a fresh QModbusDataUnit instance or reply for coils returns empty
		//        wierdly, registers work fine when passing m_modbusDataUnit
		m_replyRead = client->m_modbusClient->sendReadRequest(
			QModbusDataUnit(m_modbusDataUnit.registerType(), m_modbusDataUnit.startAddress(), m_modbusDataUnit.valueCount())
			, serverAddress);
		// check if no error
		if (!m_replyRead)
		{
			return;
		}
		// check if finished immediately
		if (m_replyRead->isFinished())
		{
			// broadcast replies return immediately
			m_replyRead->deleteLater();
			m_replyRead = nullptr;
		}
		// subscribe to finished
		QObject::connect(m_replyRead, &QModbusReply::finished, this, [this]() {
			// check if reply still valid
			if (!m_replyRead)
			{
				return;
			}
			// handle error
			auto error = m_replyRead->error();
			this->lastError()->setValue(error);
			if (error != QModbusDevice::Error::NoError)
			{
				// TODO : send UA event
			}
			// update block value
			QVector<quint16> vectValues = m_replyRead->result().values();
			this->data()->setValue(QVariant::fromValue(vectValues));
			// update modbus values
			auto values = this->values()->values();
			for (int i = 0; i < values.count(); i++)
			{
				values.at(i)->updateValue(vectValues);
			}
			// delete reply on next event loop exec
			m_replyRead->deleteLater();
			m_replyRead = nullptr;
		}, Qt::QueuedConnection);

	}, samplingTime);
}
