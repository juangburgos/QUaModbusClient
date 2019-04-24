#include "quamodbusdatablock.h"
#include "quamodbusclient.h"
#include "quamodbusvalue.h"

quint32 QUaModbusDataBlock::m_minSamplingTime = 50;

QUaModbusDataBlock::QUaModbusDataBlock(QUaServer *server)
	: QUaBaseObject(server)
{
	m_loopHandle = -1;
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
	lastError   ()->setValue(QModbusDevice::Error::ConfigurationError);
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
	// to safely update error in ua server thread
	QObject::connect(this, &QUaModbusDataBlock::updateLastError, this, &QUaModbusDataBlock::on_updateLastError);
	// set descriptions
	type        ()->setDescription("Type of Modbus register for this block.");
	address     ()->setDescription("Start register address for this block (with respect to the register type).");
	size        ()->setDescription("Size (in registers) for this block.");
	samplingTime()->setDescription("Polling time (cycle time) to read this block.");
	data        ()->setDescription("The current block values as per the last successfull read.");
	lastError   ()->setDescription("The last error reported while reading or writing this block.");
	values      ()->setDescription("List of converted values.");
}

QUaProperty * QUaModbusDataBlock::type() const
{
	return this->browseChild<QUaProperty>("Type");
}

QUaProperty * QUaModbusDataBlock::address() const
{
	return this->browseChild<QUaProperty>("Address");
}

QUaProperty * QUaModbusDataBlock::size() const
{
	return this->browseChild<QUaProperty>("Size");
}

QUaProperty * QUaModbusDataBlock::samplingTime() const
{
	return this->browseChild<QUaProperty>("SamplingTime");
}

QUaBaseDataVariable * QUaModbusDataBlock::data() const
{
	return this->browseChild<QUaBaseDataVariable>("Data");
}

QUaBaseDataVariable * QUaModbusDataBlock::lastError() const
{
	return this->browseChild<QUaBaseDataVariable>("LastError");
}

QUaModbusValueList * QUaModbusDataBlock::values() const
{
	return this->browseChild<QUaModbusValueList>("Values");
}

void QUaModbusDataBlock::remove()
{
	// stop loop
	this->client()->m_workerThread.stopLoopInThread(m_loopHandle);
	m_loopHandle = -1;
	// call deleteLater in thread, so thread has time to stop loop first
	// NOTE : deleteLater will delete the object in the correct thread anyways
	this->client()->m_workerThread.execInThread([this]() {
		// then delete
		this->deleteLater();	
	}, Qt::EventPriority::LowEventPriority);
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
	m_loopHandle = -1;
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
	QVector<quint16> data = QUaModbusDataBlock::variantToInt16Vect(value);
	// exec write request in client thread
	this->client()->m_workerThread.execInThread([this, data]() {
		auto client = this->client();
		// check if request is valid
		if (m_modbusDataUnit.registerType() != QModbusDataUnit::RegisterType::Coils &&
			m_modbusDataUnit.registerType() != QModbusDataUnit::RegisterType::HoldingRegisters)
		{
			return;
		}
		if (m_modbusDataUnit.startAddress() < 0)
		{
			emit this->updateLastError(QModbusDevice::Error::ConfigurationError);
			return;
		}
		if (m_modbusDataUnit.valueCount() == 0)
		{
			emit this->updateLastError(QModbusDevice::Error::ConfigurationError);
			return;
		}
		// check if connected
		auto state = client->state()->value().value<QModbusDevice::State>();
		if (state != QModbusDevice::State::ConnectedState)
		{
			emit this->updateLastError(QModbusDevice::Error::ConnectionError);
			return;
		}
		// create data target 
		QModbusDataUnit dataToWrite(m_modbusDataUnit.registerType(), m_modbusDataUnit.startAddress(), data);
		// create and send request
		auto serverAddress = client->serverAddress()->value().value<quint8>();
		QModbusReply * p_reply = client->m_modbusClient->sendWriteRequest(dataToWrite, serverAddress);
		if (!p_reply)
		{
			emit this->updateLastError(QModbusDevice::Error::ReplyAbortedError);
			return;
		}
		// subscribe to finished
		QObject::connect(p_reply, &QModbusReply::finished, this, [this, p_reply]() mutable {
			// NOTE : exec'd in ua server thread (not in worker thread)
			// check if reply still valid
			if (!p_reply)
			{
				lastError()->setValue(QModbusDevice::Error::ReplyAbortedError);
				return;
			}
			// handle error
			auto error = p_reply->error();
			this->lastError()->setValue(error);
			// update values errors
			auto values = this->values()->values();
			for (int i = 0; i < values.count(); i++)
			{
				values.at(i)->lastError()->setValue(error);
			}
			// delete reply on next event loop exec
			p_reply->deleteLater();
			p_reply = nullptr;
		}, Qt::QueuedConnection);
	});
}

void QUaModbusDataBlock::on_updateLastError(const QModbusDevice::Error & error)
{
	lastError()->setValue(error);
	// check if need to check errors in values
	if (error != QModbusDevice::Error::ConnectionError)
	{
		return;
	}
	// update errors in values
	auto values = this->values()->values();
	for (int i = 0; i < values.count(); i++)
	{
		auto oldValErr = values.at(i)->lastError()->value().value<QModbusDevice::Error>();
		if (oldValErr != QModbusDevice::Error::ConfigurationError)
		{
			values.at(i)->lastError()->setValue(QModbusDevice::Error::ConnectionError);
		}
	}
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
		// check if request is valid
		if (m_modbusDataUnit.registerType() == QModbusDataUnit::RegisterType::Invalid)
		{
			emit this->updateLastError(QModbusDevice::Error::ConfigurationError);
			return;
		}
		if (m_modbusDataUnit.startAddress() < 0)
		{
			emit this->updateLastError(QModbusDevice::Error::ConfigurationError);
			return;
		}
		if (m_modbusDataUnit.valueCount() == 0)
		{
			emit this->updateLastError(QModbusDevice::Error::ConfigurationError);
			return;
		}
		// check if connected
		auto state = client->state()->value().value<QModbusDevice::State>();
		if (state != QModbusDevice::State::ConnectedState)
		{
			emit this->updateLastError(QModbusDevice::Error::ConnectionError);
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
			emit this->updateLastError(QModbusDevice::Error::ReplyAbortedError);
			return;
		}
		// check if finished immediately (ignore)
		if (m_replyRead->isFinished())
		{
			// broadcast replies return immediately
			m_replyRead->deleteLater();
			m_replyRead = nullptr;
			return;
		}
		// subscribe to finished
		QObject::connect(m_replyRead, &QModbusReply::finished, this, [this]() {
			// NOTE : exec'd in ua server thread (not in worker thread)
			// check if reply still valid
			if (!m_replyRead)
			{
				lastError()->setValue(QModbusDevice::Error::ReplyAbortedError);
				return;
			}
			// handle error
			auto error = m_replyRead->error();
			this->lastError()->setValue(error);
			// update block value
			QVector<quint16> vectValues = m_replyRead->result().values();
			this->data()->setValue(QVariant::fromValue(vectValues));
			// update modbus values and errors
			auto values = this->values()->values();
			for (int i = 0; i < values.count(); i++)
			{
				values.at(i)->setValue(vectValues, error);
			}
			// delete reply on next event loop exec
			m_replyRead->deleteLater();
			m_replyRead = nullptr;
		}, Qt::QueuedConnection);

	}, samplingTime);
}

bool QUaModbusDataBlock::loopRunning()
{
	return m_loopHandle >= 0;
}

QDomElement QUaModbusDataBlock::toDomElement(QDomDocument & domDoc) const
{
	// add block element
	QDomElement elemBlock = domDoc.createElement(QUaModbusDataBlock::staticMetaObject.className());
	// set block attributes
	elemBlock.setAttribute("BrowseName"  , this->browseName());
	elemBlock.setAttribute("Type"        , QMetaEnum::fromType<QUaModbusDataBlock::RegisterType>().valueToKey(type()->value().value<QUaModbusDataBlock::RegisterType>()));
	elemBlock.setAttribute("Address"     , address()->value().toInt());
	elemBlock.setAttribute("Size"        , size()->value().toUInt());
	elemBlock.setAttribute("SamplingTime", samplingTime()->value().toUInt());
	// add value list element
	auto elemValueList = values()->toDomElement(domDoc);
	elemBlock.appendChild(elemValueList);
	// return block element
	return elemBlock;
}

void QUaModbusDataBlock::fromDomElement(QDomElement & domElem, QString & strError)
{
	// get client attributes (BrowseName must be already set)
	QString strBrowseName = domElem.attribute("BrowseName", "");
	Q_ASSERT(browseName().compare(strBrowseName, Qt::CaseInsensitive) == 0);
	bool bOK;
	// Type
	auto type = QMetaEnum::fromType<QUaModbusDataBlock::RegisterType>().keysToValue(domElem.attribute("Type").toUtf8(), &bOK);
	if (bOK)
	{
		this->type()->setValue(type);
		// NOTE : force internal update
		this->on_typeChanged(type);
	}
	else
	{
		strError += QString("Error : Invalid Type attribute '%1' in Block %2. Ignoring.\n").arg(type).arg(strBrowseName);
	}
	
	// Address
	auto address = domElem.attribute("Address").toInt(&bOK);
	if (bOK)
	{
		this->address()->setValue(address);
		// NOTE : force internal update
		this->on_addressChanged(address);
	}
	else
	{
		strError += QString("Error : Invalid Address attribute '%1' in Block %2. Ignoring.\n").arg(address).arg(strBrowseName);
	}
	// Size
	auto size = domElem.attribute("Size").toUInt(&bOK);
	if (bOK)
	{
		this->size()->setValue(size);
		// NOTE : force internal update
		this->on_sizeChanged(size);
	}
	else
	{
		strError += QString("Error : Invalid Size attribute '%1' in Block %2. Ignoring.\n").arg(size).arg(strBrowseName);
	}
	// SamplingTime
	auto samplingTime = domElem.attribute("SamplingTime").toUInt(&bOK);
	if (bOK)
	{
		this->samplingTime()->setValue(samplingTime);
		// NOTE : force internal update
		this->on_samplingTimeChanged(samplingTime);
	}
	else
	{
		strError += QString("Error : Invalid SamplingTime attribute '%1' in Block %2. Ignoring.\n").arg(samplingTime).arg(strBrowseName);
	}
	// get value list
	QDomElement elemValueList = domElem.firstChildElement(QUaModbusValueList::staticMetaObject.className());
	if (!elemValueList.isNull())
	{
		values()->fromDomElement(elemValueList, strError);
	}
	else
	{
		strError += QString("Error : Block %1 does not have a QUaModbusValueList child. No values will be loaded.\n").arg(strBrowseName);
	}
}

QVector<quint16> QUaModbusDataBlock::variantToInt16Vect(const QVariant & value)
{
	QVector<quint16> data;
	// check if valid
	if (!value.isValid())
	{
		return data;
	}
	// convert
	QSequentialIterable iterable = value.value<QSequentialIterable>();
	QSequentialIterable::const_iterator it = iterable.begin();
	const QSequentialIterable::const_iterator end = iterable.end();
	for (; it != end; ++it) {
		data << (*it).value<quint16>();
	}
	return data;
}
