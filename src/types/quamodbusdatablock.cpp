#include "quamodbusdatablock.h"
#include "quamodbusclient.h"
#include "quamodbusvalue.h"

#ifdef QUA_ACCESS_CONTROL
#include <QUaPermissions>
#endif // QUA_ACCESS_CONTROL

quint32 QUaModbusDataBlock::m_minSamplingTime = 50;

QUaModbusDataBlock::QUaModbusDataBlock(QUaServer *server)
#ifndef QUA_ACCESS_CONTROL
	: QUaBaseObject(server)
#else
	: QUaBaseObjectProtected(server)
#endif // !QUA_ACCESS_CONTROL
{
	m_loopHandle = -1;
	m_replyRead  = nullptr;
	// NOTE : QObject parent might not be yet available in constructor
	type   ()->setDataTypeEnum(QMetaEnum::fromType<QModbusDataBlockType>());
	type   ()->setValue(QModbusDataBlockType::Invalid);
	address()->setDataType(QMetaType::Int);
	address()->setValue(-1);
	size   ()->setDataType(QMetaType::UInt);
	size   ()->setValue(0);
	samplingTime()->setDataType(QMetaType::UInt);
	samplingTime()->setValue(1000);
	lastError   ()->setDataTypeEnum(QMetaEnum::fromType<QModbusError>());
	lastError   ()->setValue(QModbusError::ConnectionError);
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
	/*
	type        ()->setDescription(tr("Type of Modbus register for this block."));
	address     ()->setDescription(tr("Start register address for this block (with respect to the register type)."));
	size        ()->setDescription(tr("Size (in registers) for this block."));
	samplingTime()->setDescription(tr("Polling time (cycle time) to read this block."));
	data        ()->setDescription(tr("The current block values as per the last successfull read."));
	lastError   ()->setDescription(tr("The last error reported while reading or writing this block."));
	values      ()->setDescription(tr("List of converted values."));
	*/
}

QUaProperty * QUaModbusDataBlock::type() const
{
	return const_cast<QUaModbusDataBlock*>(this)->browseChild<QUaProperty>("Type");
}

QUaProperty * QUaModbusDataBlock::address() const
{
	return const_cast<QUaModbusDataBlock*>(this)->browseChild<QUaProperty>("Address");
}

QUaProperty * QUaModbusDataBlock::size() const
{
	return const_cast<QUaModbusDataBlock*>(this)->browseChild<QUaProperty>("Size");
}

QUaProperty * QUaModbusDataBlock::samplingTime() const
{
	return const_cast<QUaModbusDataBlock*>(this)->browseChild<QUaProperty>("SamplingTime");
}

QUaBaseDataVariable * QUaModbusDataBlock::data() const
{
	return const_cast<QUaModbusDataBlock*>(this)->browseChild<QUaBaseDataVariable>("Data");
}

QUaBaseDataVariable * QUaModbusDataBlock::lastError() const
{
	return const_cast<QUaModbusDataBlock*>(this)->browseChild<QUaBaseDataVariable>("LastError");
}

QUaModbusValueList * QUaModbusDataBlock::values() const
{
	return const_cast<QUaModbusDataBlock*>(this)->browseChild<QUaModbusValueList>("Values");
}

void QUaModbusDataBlock::remove()
{
	// stop loop
	this->client()->m_workerThread.stopLoopInThread(m_loopHandle);
	// make handle invalid **after** stopping loop in thread
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
	auto type = value.value<QModbusDataBlockType>();
	// set in thread for safety
	this->client()->m_workerThread.execInThread([this, type]() {
		m_modbusDataUnit.setRegisterType((QModbusDataUnit::RegisterType)type);
	});
	// set data writable according to type
	if (type == QModbusDataBlockType::Coils ||
		type == QModbusDataBlockType::HoldingRegisters)
	{
		data()->setWriteAccess(true);
	}
	else
	{
		data()->setWriteAccess(false);
	}
	// emit
	emit this->typeChanged(type);
}

void QUaModbusDataBlock::on_addressChanged(const QVariant & value)
{
	auto address = value.value<int>();
	// set in thread for safety
	this->client()->m_workerThread.execInThread([this, address]() {
		m_modbusDataUnit.setStartAddress(address);
	});
	// emit
	emit this->addressChanged(address);
}

void QUaModbusDataBlock::on_sizeChanged(const QVariant & value)
{
	auto size = value.value<quint32>();
	// set in thread for safety
	this->client()->m_workerThread.execInThread([this, size]() {
		m_modbusDataUnit.setValueCount(size);
	});
	// emit
	emit this->sizeChanged(size);
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
		// emit
		emit this->samplingTimeChanged(QUaModbusDataBlock::m_minSamplingTime);
		return;
	}
	// stop old loop
	this->client()->m_workerThread.stopLoopInThread(m_loopHandle);
	// make handle invalid **after** stopping loop in thread
	m_loopHandle = -1;
	// start new loop
	this->startLoop();
	// update ua sample interval for data
	this->data()->setMinimumSamplingInterval((double)samplingTime);
	// emit
	emit this->samplingTimeChanged(samplingTime);
}

void QUaModbusDataBlock::on_dataChanged(const QVariant & value)
{
	// convert data
	QVector<quint16> data = QUaModbusDataBlock::variantToInt16Vect(value);
	// emit
	emit this->dataChanged(data);
	// write to modbus
	this->setModbusData(data);
}

void QUaModbusDataBlock::on_updateLastError(const QModbusError & error)
{
	// avoid update or emit if no change, improves performance
	if (error == this->getLastError())
	{
		return;
	}
	this->lastError()->setValue(error);
	// NOTE : need to add custom signal because OPC UA valueChanged
	//        only works for changes through network
	// emit
	emit this->lastErrorChanged(error);
	// update errors in values
	auto values = this->values()->values();
	for (auto value : values)
	{
		auto oldValErr = value->getLastError();
		if (error == QModbusError::ConnectionError && oldValErr != QModbusError::ConfigurationError)
		{
			value->setLastError(QModbusError::ConnectionError);
		}
		else if (error != QModbusError::ConnectionError && oldValErr == QModbusError::NoError)
		{
			value->setLastError(error);
		}
	}
}

QUaModbusDataBlockList * QUaModbusDataBlock::list() const
{
	return qobject_cast<QUaModbusDataBlockList*>(this->parent());
}

QUaModbusClient * QUaModbusDataBlock::client() const
{
	return this->list()->client();
}

void QUaModbusDataBlock::startLoop()
{
	auto samplingTime = this->samplingTime()->value().value<quint32>();
	// exec read request in client thread
	m_loopHandle = this->client()->m_workerThread.startLoopInThread(
	[this]() {
		//Q_ASSERT(m_loopHandle > 0); // NOTE : this does happen when cleaning all blocks form a client
		if (m_loopHandle <= 0)
		{
			return;
		}
		auto client = this->client();
		// TODO : can happen in shutdown? possible BUG
		if (!client)
		{
			return;
		}
		// check if ongoing request
		if (m_replyRead)
		{
			return;
		}
		// check if request is valid
		if (m_modbusDataUnit.registerType() == QModbusDataBlockType::Invalid)
		{
			emit this->updateLastError(QModbusError::ConfigurationError);
			return;
		}
		if (m_modbusDataUnit.startAddress() < 0)
		{
			emit this->updateLastError(QModbusError::ConfigurationError);
			return;
		}
		if (m_modbusDataUnit.valueCount() == 0)
		{
			emit this->updateLastError(QModbusError::ConfigurationError);
			return;
		}
		// check if connected
		auto state = client->getState();
		if (state != QModbusState::ConnectedState)
		{
			emit this->updateLastError(QModbusError::ConnectionError);
			return;
		}
		// create and send request		
		auto serverAddress = client->getServerAddress();
		// NOTE : need to pass in a fresh QModbusDataUnit instance or reply for coils returns empty
		//        wierdly, registers work fine when passing m_modbusDataUnit
		m_replyRead = client->m_modbusClient->sendReadRequest(
			QModbusDataUnit(m_modbusDataUnit.registerType(), m_modbusDataUnit.startAddress(), m_modbusDataUnit.valueCount())
			, serverAddress);
		// check if no error
		if (!m_replyRead)
		{
			emit this->updateLastError(QModbusError::ReplyAbortedError);
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
		QObject::connect(m_replyRead, &QModbusReply::finished, this, 
		[this]() {
			// NOTE : exec'd in ua server thread (not in worker thread)
			// check if reply still valid
			if (!m_replyRead)
			{
				this->setLastError(QModbusError::ReplyAbortedError);
				return;
			}
			// handle error
			auto error = m_replyRead->error();
			this->setLastError(error);
			// update block value
			QVector<quint16> data = m_replyRead->result().values();
			// TODO : early exit when refactor QUaModbusValue::setValue
			if (error == QModbusError::NoError)
			{
				Q_ASSERT(data.count() == m_modbusDataUnit.valueCount());
				this->setData(data, false);
			}
			// update modbus values and errors
			auto values = this->values()->values();
			for (auto value : values)
			{
				value->setValue(data, error);
			}
			// delete reply on next event loop exec
			m_replyRead->deleteLater();
			m_replyRead = nullptr;
		}, Qt::QueuedConnection);
	}, samplingTime);
	Q_ASSERT(m_loopHandle > 0);
}

bool QUaModbusDataBlock::loopRunning()
{
	return m_loopHandle >= 0;
}

void QUaModbusDataBlock::setModbusData(const QVector<quint16>& data)
{
	// exec write request in client thread
	this->client()->m_workerThread.execInThread(
	[this, data]() {
		auto client = this->client();
		// check if request is valid
		if (m_modbusDataUnit.registerType() != QModbusDataBlockType::Coils &&
			m_modbusDataUnit.registerType() != QModbusDataBlockType::HoldingRegisters)
		{
			return;
		}
		if (m_modbusDataUnit.startAddress() < 0)
		{
			emit this->updateLastError(QModbusError::ConfigurationError);
			return;
		}
		if (m_modbusDataUnit.valueCount() == 0)
		{
			emit this->updateLastError(QModbusError::ConfigurationError);
			return;
		}
		// check if connected
		auto state = client->getState();
		if (state != QModbusState::ConnectedState)
		{
			emit this->updateLastError(QModbusError::ConnectionError);
			return;
		}
		// create data target 
		QModbusDataUnit dataToWrite(m_modbusDataUnit.registerType(), m_modbusDataUnit.startAddress(), data);
		// create and send request
		auto serverAddress = client->getServerAddress();
		QModbusReply * p_reply = client->m_modbusClient->sendWriteRequest(dataToWrite, serverAddress);
		if (!p_reply)
		{
			emit this->updateLastError(QModbusError::ReplyAbortedError);
			return;
		}
		// subscribe to finished
		QObject::connect(p_reply, &QModbusReply::finished, this, 
		[this, p_reply]() mutable {
			// NOTE : exec'd in ua server thread (not in worker thread)
			// check if reply still valid
			if (!p_reply)
			{
				auto error = QModbusError::ReplyAbortedError;
				this->setLastError(error);
				return;
			}
			// handle error
			auto error = p_reply->error();
			this->setLastError(error);
			// delete reply on next event loop exec
			p_reply->deleteLater();
			p_reply = nullptr;
		}, Qt::QueuedConnection);
	});
}

QDomElement QUaModbusDataBlock::toDomElement(QDomDocument & domDoc) const
{
	// add block element
	QDomElement elemBlock = domDoc.createElement(QUaModbusDataBlock::staticMetaObject.className());
#ifdef QUA_ACCESS_CONTROL
	// set parmissions if any
	if (this->hasPermissionsObject())
	{
		elemBlock.setAttribute("Permissions", this->permissionsObject()->nodeId());
	}
#endif // QUA_ACCESS_CONTROL
	// set block attributes
	elemBlock.setAttribute("BrowseName"  , this->browseName().name());
	elemBlock.setAttribute("Type"        , QMetaEnum::fromType<QModbusDataBlockType>().valueToKey(getType()));
	elemBlock.setAttribute("Address"     , getAddress());
	elemBlock.setAttribute("Size"        , getSize());
	elemBlock.setAttribute("SamplingTime", getSamplingTime());
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
	Q_ASSERT(this->browseName() == QUaQualifiedName(strBrowseName));
#ifdef QUA_ACCESS_CONTROL
	// load permissions if any
	if (domElem.hasAttribute("Permissions") && !domElem.attribute("Permissions").isEmpty())
	{
		strError += this->setPermissions(domElem.attribute("Permissions"));
	}
#endif // QUA_ACCESS_CONTROL
	bool bOK;
	// Type
	auto type = QMetaEnum::fromType<QModbusDataBlockType>().keysToValue(domElem.attribute("Type").toUtf8(), &bOK);
	if (bOK)
	{
		this->type()->setValue(type);
		// NOTE : force internal update
		this->on_typeChanged(type);
	}
	else
	{
		strError += tr("%1 : Invalid Type attribute '%2' in Block %3. Default value set.\n").arg("Warning").arg(type).arg(strBrowseName);
	}
	
	// Address
	auto address = domElem.attribute("Address").toInt(&bOK);
	if (bOK)
	{
		this->setAddress(address);
	}
	else
	{
		strError += tr("%1 : Invalid Address attribute '%2' in Block %3. Default value set.\n").arg("Warning").arg(address).arg(strBrowseName);
	}
	// Size
	auto size = domElem.attribute("Size").toUInt(&bOK);
	if (bOK)
	{
		this->setSize(size);
	}
	else
	{
		strError += tr("%1 : Invalid Size attribute '%2' in Block %3. Default value set.\n").arg("Warning").arg(size).arg(strBrowseName);
	}
	// SamplingTime
	auto samplingTime = domElem.attribute("SamplingTime").toUInt(&bOK);
	if (bOK)
	{
		this->setSamplingTime(samplingTime);
	}
	else
	{
		strError += tr("%1 : Invalid SamplingTime attribute '%2' in Block %3. Default value set.\n").arg("Warning").arg(samplingTime).arg(strBrowseName);
	}
	// get value list
	QDomElement elemValueList = domElem.firstChildElement(QUaModbusValueList::staticMetaObject.className());
	if (!elemValueList.isNull())
	{
		values()->fromDomElement(elemValueList, strError);
	}
	else
	{
		strError += tr("%1 : Block %2 does not have a QUaModbusValueList child. No values will be loaded.\n").arg("Warning").arg(strBrowseName);
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

QModbusDataBlockType QUaModbusDataBlock::getType() const
{
	return this->type()->value().value<QModbusDataBlockType>();
}

void QUaModbusDataBlock::setType(const QModbusDataBlockType & type)
{
	this->type()->setValue(type);
	this->on_typeChanged(type);
}

int QUaModbusDataBlock::getAddress() const
{
	return this->address()->value().toInt();
}

void QUaModbusDataBlock::setAddress(const int & address)
{
	this->address()->setValue(address);
	this->on_addressChanged(address);
}

quint32 QUaModbusDataBlock::getSize() const
{
	return this->size()->value().value<quint32>();
}

void QUaModbusDataBlock::setSize(const quint32 & size)
{
	this->size()->setValue(size);
	this->on_sizeChanged(size);
}

quint32 QUaModbusDataBlock::getSamplingTime() const
{
	return this->samplingTime()->value().value<quint32>();
}

void QUaModbusDataBlock::setSamplingTime(const quint32 & samplingTime)
{
	this->samplingTime()->setValue(samplingTime);
	this->on_samplingTimeChanged(samplingTime);
}

QVector<quint16> QUaModbusDataBlock::getData() const
{
	return QUaModbusDataBlock::variantToInt16Vect(this->data()->value());
}

void QUaModbusDataBlock::setData(const QVector<quint16>& data, const bool &writeModbus/* = true*/)
{
	Q_ASSERT_X(data.count() == this->getSize(), "QUaModbusDataBlock::setData", "Received block of incorrect size");
	auto varData = QVariant::fromValue(data);
	// set on OPC
	this->data()->setValue(varData); // TODO : check of memory leak when writing array
	// emit change c++
	emit this->dataChanged(data);
	// check if write to modbus
	if (!writeModbus)
	{
		return;
	}
	this->setModbusData(data);
}

QModbusError QUaModbusDataBlock::getLastError() const
{
	return this->lastError()->value().value<QModbusError>();
}

void QUaModbusDataBlock::setLastError(const QModbusError & error)
{
	// call internal slot on_updateLastError
	emit this->updateLastError(error);
}

