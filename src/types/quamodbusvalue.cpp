#include "quamodbusclient.h"
#include "quamodbusvalue.h"
#include "quamodbusvaluelist.h"
#include "quamodbusdatablock.h"

#include <QUaProperty>
#include <QUaBaseDataVariable>

#ifdef QUA_ACCESS_CONTROL
#include <QUaPermissions>
#endif // QUA_ACCESS_CONTROL

QUaModbusValue::QUaModbusValue(QUaServer *server)
#ifndef QUA_ACCESS_CONTROL
	: QUaBaseObject(server)
#else
	: QUaBaseObjectProtected(server)
#endif // !QUA_ACCESS_CONTROL
{
	// set defaults
	m_loopId = 0;
	m_type = nullptr;
	m_registersUsed = nullptr;
	m_addressOffset = nullptr;
#ifndef QUAMODBUS_NOCYCLIC_WRITE
	m_cyclicWritePeriod = nullptr;
	m_cyclicWriteMode = nullptr;
#endif // !QUAMODBUS_NOCYCLIC_WRITE
	m_value = nullptr;
	m_lastError = nullptr;
	type             ()->setDataTypeEnum(QMetaEnum::fromType<QModbusValueType>());
	type             ()->setValue(QModbusValueType::Invalid);
	registersUsed    ()->setDataType(QMetaType::UShort);
	registersUsed    ()->setValue(0);
	addressOffset    ()->setDataType(QMetaType::Int);
	addressOffset    ()->setValue(-1);
	lastError        ()->setDataTypeEnum(QMetaEnum::fromType<QModbusError>());
	lastError        ()->setValue(QModbusError::ConfigurationError);
	// set initial conditions
	type             ()->setWriteAccess(true);
	addressOffset    ()->setWriteAccess(true);
	value            ()->setWriteAccess(false); // set to true, when type != ValueType::Invalid
	// handle state changes
	QObject::connect(type()             , &QUaBaseVariable::valueChanged, this, &QUaModbusValue::on_typeChanged             , Qt::QueuedConnection);
	QObject::connect(addressOffset()    , &QUaBaseVariable::valueChanged, this, &QUaModbusValue::on_addressOffsetChanged    , Qt::QueuedConnection);
	QObject::connect(value()            , &QUaBaseVariable::valueChanged, this, &QUaModbusValue::on_valueChanged            , Qt::QueuedConnection);
	// to safely update error in ua server thread
	QObject::connect(this, &QUaModbusValue::updateLastError, this, &QUaModbusValue::on_updateLastError);

	// set descriptions
	/*
	type()         ->setDescription(tr("Data type used to convert the registers to the value."));
	registersUsed()->setDescription(tr("Number of registeres used by the selected data type"));
	addressOffset()->setDescription(tr("Offset with respect to the data block."));
	value()        ->setDescription(tr("The value obtained by converting the registers to the selected type."));
	lastError()    ->setDescription(tr("Last error obtained while converting registers to value."));
	*/

#ifndef QUAMODBUS_NOCYCLIC_WRITE
	cyclicWritePeriod()->setDataType(QMetaType::UInt);
	cyclicWritePeriod()->setValue(0);
	cyclicWriteMode()->setDataTypeEnum(QMetaEnum::fromType<QModbusCyclicWriteMode>());
	cyclicWriteMode()->setValue(QModbusCyclicWriteMode::Current);
	QObject::connect(cyclicWritePeriod(), &QUaBaseVariable::valueChanged, this, &QUaModbusValue::on_cyclicWritePeriodChanged, Qt::QueuedConnection);
	QObject::connect(cyclicWriteMode()  , &QUaBaseVariable::valueChanged, this, &QUaModbusValue::on_cyclicWriteModeChanged  , Qt::QueuedConnection);
	QObject::connect(this, &QUaModbusValue::cyclicWrite    , this, &QUaModbusValue::on_cyclicWrite    );
	cyclicWritePeriod()->setWriteAccess(true);
	cyclicWriteMode()->setWriteAccess(true);
#endif // !QUAMODBUS_NOCYCLIC_WRITE
}

QUaProperty * QUaModbusValue::type()
{
	if (!m_type)
	{
		m_type = this->browseChild<QUaProperty>("Type");
	}
	return m_type;
}

QUaProperty * QUaModbusValue::registersUsed()
{
	if (!m_registersUsed)
	{
		m_registersUsed = this->browseChild<QUaProperty>("RegistersUsed");
	}
	return m_registersUsed;
}

QUaProperty * QUaModbusValue::addressOffset()
{
	if (!m_addressOffset)
	{
		m_addressOffset = this->browseChild<QUaProperty>("AddressOffset");
	}
	return m_addressOffset;
}

#ifndef QUAMODBUS_NOCYCLIC_WRITE
QUaProperty* QUaModbusValue::cyclicWritePeriod()
{
	if (!m_cyclicWritePeriod)
	{
		m_cyclicWritePeriod = this->browseChild<QUaProperty>("CyclicWritePeriod");
	}
	return m_cyclicWritePeriod;
}

QUaProperty* QUaModbusValue::cyclicWriteMode()
{
	if (!m_cyclicWriteMode)
	{
		m_cyclicWriteMode = this->browseChild<QUaProperty>("CyclicWriteMode");
	}
	return m_cyclicWriteMode;
}

quint32 QUaModbusValue::getCyclicWritePeriod() const
{
	return const_cast<QUaModbusValue*>(this)->cyclicWritePeriod()->value<quint32>();
}

void QUaModbusValue::setCyclicWritePeriod(const quint32& cyclicWritePeriod)
{
	this->cyclicWritePeriod()->setValue(cyclicWritePeriod);
	this->on_cyclicWritePeriodChanged(cyclicWritePeriod, true);
}

QModbusCyclicWriteMode QUaModbusValue::getCyclicWriteMode() const
{
	return const_cast<QUaModbusValue*>(this)->cyclicWriteMode()->value().value<QModbusCyclicWriteMode>();
}

void QUaModbusValue::setCyclicWriteMode(const QModbusCyclicWriteMode& cyclicWriteMode)
{
	this->cyclicWriteMode()->setValue(cyclicWriteMode);
	this->on_cyclicWriteModeChanged(cyclicWriteMode, true);
}

void QUaModbusValue::on_cyclicWritePeriodChanged(const QVariant& value, const bool& networkChange)
{
	if (!networkChange)
	{
		return;
	}
	// stop previous loop
	if (m_loopId != 0)
	{
		this->client()->m_workerThread.stopLoopInThread(m_loopId);
	}
	quint32 cyclePeriod = value.value<quint32>();
	// emit
	emit this->cyclicWritePeriodChanged(cyclePeriod);
	// exit if no need to start another loop
	if (cyclePeriod == 0)
	{
		return;
	}
	m_loopId = this->client()->m_workerThread.startLoopInThread(
	[this]() {
		auto state = this->client()->getState();
		if (state != QModbusState::ConnectedState)
		{
			return;
		}
		emit this->cyclicWrite();
	}, 
	cyclePeriod);
}

void QUaModbusValue::on_cyclicWriteModeChanged(const QVariant& value, const bool& networkChange)
{
	if (!networkChange)
	{
		return;
	}
	emit this->cyclicWriteModeChanged(value.value<QModbusCyclicWriteMode>());
}

void QUaModbusValue::on_cyclicWrite()
{
	auto value = this->getValue();
	// cyclic write logic
	auto mode = this->getCyclicWriteMode();
	switch (mode)
	{
	case QModbusCyclicWriteMode::Current:
		break;
	case QModbusCyclicWriteMode::Toggle:
		{
			bool val = value.value<bool>();
			value.setValue(!val);
		}
		break;
	case QModbusCyclicWriteMode::Increase:
		{
			int val = value.value<int>();
			value.setValue(++val);
		}
		break;
	case QModbusCyclicWriteMode::Decrease:
		{
			int val = value.value<int>();
			value.setValue(--val);
		}
		break;
	default:
		break;
	}
	// actually write
	this->on_valueChanged(value, true);
}
#endif // !QUAMODBUS_NOCYCLIC_WRITE

QUaBaseDataVariable * QUaModbusValue::value()
{
	if (!m_value)
	{
		m_value = this->browseChild<QUaBaseDataVariable>("Value");
	}
	return m_value;
}

QUaBaseDataVariable * QUaModbusValue::lastError()
{
	if (!m_lastError)
	{
		m_lastError = this->browseChild<QUaBaseDataVariable>("LastError");
	}
	return m_lastError;
}

void QUaModbusValue::remove()
{
	this->deleteLater();
}

void QUaModbusValue::on_typeChanged(const QVariant &value, const bool& networkChange)
{
	if (!networkChange)
	{
		return;
	}
	auto type = value.value<QModbusValueType>();
	// convert to metatype to set as UA type
	auto metaType = QUaModbusValue::typeToMeta(type);
	this->value()->setDataType(metaType);
	// update value is possible
	auto blockError   = this->block()->getLastError();
	auto blockData    = this->block()->getData();
	this->setValue(blockData, blockError);
	// update number of registers used
	auto registersUsed = QUaModbusValue::typeBlockSize(type);
	this->registersUsed()->setValue(registersUsed);
	// emit
	emit this->typeChanged(type);
	emit this->registersUsedChanged(registersUsed);
}

QModbusValueType QUaModbusValue::getType() const
{
	return const_cast<QUaModbusValue*>(this)->type()->value().value<QModbusValueType>();
}

void QUaModbusValue::setType(const QModbusValueType & type)
{
	if (this->getType() == type)
	{
		return;
	}
	this->type()->setValue(type);
	this->on_typeChanged(type, true);
}

quint16 QUaModbusValue::getRegistersUsed() const
{
	return const_cast<QUaModbusValue*>(this)->registersUsed()->value().value<quint16>();
}

int QUaModbusValue::getAddressOffset() const
{
	return const_cast<QUaModbusValue*>(this)->addressOffset()->value().toInt();
}

void QUaModbusValue::setAddressOffset(const int & addressOffset)
{
	this->addressOffset()->setValue(addressOffset);
	this->on_addressOffsetChanged(addressOffset, true);
}

QVariant QUaModbusValue::getValue() const
{
	return const_cast<QUaModbusValue*>(this)->value()->value();
}

// programmatic change from C++ API
void QUaModbusValue::setValue(const QVariant & value)
{
	this->value()->setValue(value);
	this->on_valueChanged(value, true);
}

QModbusError QUaModbusValue::getLastError() const
{
	return const_cast<QUaModbusValue*>(this)->lastError()->value().value<QModbusError>();
}

void QUaModbusValue::setLastError(const QModbusError & error)
{
	// call internal slot on_updateLastError
	emit this->updateLastError(error);
}

bool QUaModbusValue::isWritable() const
{
	auto block = this->block();
	auto type  = block->getType();
	return type == QModbusDataBlockType::Coils || 
		   type == QModbusDataBlockType::HoldingRegisters;
}

void QUaModbusValue::on_addressOffsetChanged(const QVariant & value, const bool& networkChange)
{
	if (!networkChange)
	{
		return;
	}
	// update value is possible
	auto blockError   = this->block()->getLastError();
	auto blockData    = this->block()->getData();
	this->setValue(blockData, blockError);
	// emit
	emit this->addressOffsetChanged(value.toInt());
}

// OPC UA network change
void QUaModbusValue::on_valueChanged(const QVariant & value, const bool& networkChange)
{
	if (!networkChange)
	{
		return;
	}
	// get block representation of value
	auto type = this->getType();
	auto data = QUaModbusValue::valueToBlock(value, type);
	// get current block
	auto blockError   = this->block()->getLastError();
	auto blockSize    = this->block()->getSize();
	// check if is connected
	if (blockError == QModbusError::ConnectionError)
	{
		this->value()->setWriteAccess(false);
		this->value()->setValue(QVariant()); // NOTE : avoid recursion
		this->setLastError(blockError);
		// emit
		emit this->valueChanged(QVariant());
		return;
	}
	// check if fits in block
	int addressOffset = this->getAddressOffset();
	if (addressOffset < 0)
	{
		this->value()->setWriteAccess(false);
		this->value()->setValue(QVariant()); // NOTE : avoid recursion
		this->setLastError(QModbusError::ConfigurationError);
		// emit
		emit this->valueChanged(QVariant());
		return;
	}
	int typeBlockSize = QUaModbusValue::typeBlockSize(type);
	if (addressOffset + typeBlockSize > static_cast<int>(blockSize))
	{
		this->value()->setWriteAccess(false);
		this->value()->setValue(QVariant()); // NOTE : avoid recursion
		this->setLastError(QModbusError::ConfigurationError);
		// emit
		emit this->valueChanged(QVariant());
		return;
	}
	// just write
	auto client = this->client();
	auto block  = this->block();
	// exec write request in client thread
	this->client()->m_workerThread.execInThread(
	[this, data, client, block, addressOffset, typeBlockSize, value]() {
		// copy from block
		auto registerType = block->m_registerType;
		auto startAddress = block->m_startAddress + addressOffset;
		auto valueCount   = typeBlockSize;
		// check if request is valid
		if (registerType != QModbusDataBlockType::Coils &&
			registerType != QModbusDataBlockType::HoldingRegisters)
		{
			return;
		}
		if (startAddress < 0)
		{
			emit this->updateLastError(QModbusError::ConfigurationError);
			return;
		}
		if (valueCount == 0)
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
		QModbusDataUnit dataToWrite(
			static_cast<QModbusDataUnit::RegisterType>(registerType),
			startAddress, 
			data
		);
		// create and send request
		auto serverAddress = client->getServerAddress();
		QModbusReply* p_reply = client->m_modbusClient->sendWriteRequest(dataToWrite, serverAddress);
		if (!p_reply)
		{
			emit this->updateLastError(QModbusError::ReplyAbortedError);
			return;
		}
		// subscribe to finished
		QObject::connect(p_reply, &QModbusReply::finished, this,
		[this, p_reply, value]() mutable {
			// NOTE : exec'd in ua server thread (not in worker thread)
			if (this->client()->m_disconnectRequested || this->client()->getState() != QModbusState::ConnectedState)
			{
				auto error = QModbusError::ReplyAbortedError;
				this->setLastError(error);
				return;
			}
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
			// emit
			emit this->valueChanged(value);
		}, Qt::QueuedConnection);
	});
}

void QUaModbusValue::on_updateLastError(const QModbusError & error)
{
	// avoid update or emit if no change, improves performance
	if (error == this->getLastError())
	{
		return;
	}
	// update
	this->lastError()->setValue(error);
	// emit
	emit this->lastErrorChanged(error);
}

// programmatic change from block upstream (modbus response to read request)
void QUaModbusValue::setValue(const QVector<quint16>& block, const QModbusError &blockError)
{
	// check configuration
	auto type = this->getType(); // TODO : change to event-based with flag
	if (type == QModbusValueType::Invalid)
	{
		this->value()->setWriteAccess(false);
		this->setLastError(QModbusError::ConfigurationError);
		return;
	}
	int addressOffset = this->getAddressOffset(); // TODO : change to event-based with flag
	if (addressOffset < 0)
	{
		this->value()->setWriteAccess(false);
		this->setLastError(QModbusError::ConfigurationError);
		return;
	}
	// set writable if block type allows it
	auto blockType = this->block()->getType(); // TODO : change to event-based with flag
	if (blockType == QUaModbusDataBlock::RegisterType::Coils ||
		blockType == QUaModbusDataBlock::RegisterType::HoldingRegisters)
	{
		this->value()->setWriteAccess(true);
	}
	else
	{
		this->value()->setWriteAccess(false);
	}
	// check if fits in block
	int typeBlockSize = QUaModbusValue::typeBlockSize(type);
	if (addressOffset + typeBlockSize > block.count())
	{
		auto newError = blockError != QModbusError::NoError ? blockError : QModbusError::ConfigurationError;
		this->setLastError(newError);
		return;
	}
	// convert value and set it, but leave block error code
	this->setLastError(blockError); // TODO : change to event-based
	// do not update value if error
	if (blockError != QModbusError::NoError)
	{
		return;
	}
	auto value = QUaModbusValue::blockToValue(block.mid(addressOffset, typeBlockSize), type);
	// avoid update or emit if no change, improves performance
	if (this->getValue() == value)
	{
		return;
	}
	// NOTE : set value before emitting to avoid recursion
	this->value()->setValue(value);
	// emit
	emit this->valueChanged(value);
}

QDomElement QUaModbusValue::toDomElement(QDomDocument & domDoc) const
{
	// add value element
	QDomElement elemValue = domDoc.createElement(QUaModbusValue::staticMetaObject.className());
#ifdef QUA_ACCESS_CONTROL
	// set parmissions if any
	if (this->hasPermissionsObject())
	{
		elemValue.setAttribute("Permissions", this->permissionsObject()->nodeId());
	}
#endif // QUA_ACCESS_CONTROL
	// set value attributes
	elemValue.setAttribute("BrowseName"   , this->browseName().name());
	elemValue.setAttribute("Type"         , QMetaEnum::fromType<QModbusValueType>().valueToKey(this->getType()));
	elemValue.setAttribute("AddressOffset", this->getAddressOffset());
#ifndef QUAMODBUS_NOCYCLIC_WRITE
	elemValue.setAttribute("CyclicWriteMode"  , QMetaEnum::fromType<QModbusCyclicWriteMode>().valueToKey(this->getCyclicWriteMode()));
	elemValue.setAttribute("CyclicWritePeriod", this->getCyclicWritePeriod());
#endif // !QUAMODBUS_NOCYCLIC_WRITE
	// return value element
	return elemValue;
}

void QUaModbusValue::fromDomElement(QDomElement & domElem, QQueue<QUaLog>& errorLogs)
{
	// get client attributes (BrowseName must be already set)
	QString strBrowseName = domElem.attribute("BrowseName", "");
	Q_ASSERT(this->browseName() == QUaQualifiedName(strBrowseName));
#ifdef QUA_ACCESS_CONTROL
	// load permissions if any
	if (domElem.hasAttribute("Permissions") && !domElem.attribute("Permissions").isEmpty())
	{
		QString strError = this->setPermissions(domElem.attribute("Permissions"));
		if (strError.contains("Error"))
		{
			errorLogs << QUaLog(
				strError,
				QUaLogLevel::Error,
				QUaLogCategory::Serialization
			);
		}
	}
#endif // QUA_ACCESS_CONTROL
	bool bOK;
	// Type
	auto type = (QModbusValueType)QMetaEnum::fromType<QModbusValueType>().keysToValue(domElem.attribute("Type").toUtf8(), &bOK);
	if (bOK)
	{
		this->setType(type);
	}
	else
	{
		errorLogs << QUaLog(
			tr("Invalid Type attribute '%1' in Value %2. Default value set.").arg("Warning").arg(type).arg(strBrowseName),
			QUaLogLevel::Warning,
			QUaLogCategory::Serialization
		);
	}
	// AddressOffset
	auto addressOffset = domElem.attribute("AddressOffset").toInt(&bOK);
	if (bOK)
	{
		this->setAddressOffset(addressOffset);
	}
	else
	{
		errorLogs << QUaLog(
			tr("Invalid AddressOffset attribute '%1' in Value %2. Default value set.").arg(addressOffset).arg(strBrowseName),
			QUaLogLevel::Warning,
			QUaLogCategory::Serialization
		);
	}
#ifndef QUAMODBUS_NOCYCLIC_WRITE
	// CyclicWriteMode
	auto mode = (QModbusCyclicWriteMode)QMetaEnum::fromType<QModbusCyclicWriteMode>().keysToValue(domElem.attribute("CyclicWriteMode").toUtf8(), &bOK);
	if (bOK)
	{
		this->setCyclicWriteMode(mode);
	}
	else
	{
		errorLogs << QUaLog(
			tr("Invalid CyclicWriteMode attribute '%1' in Value %2. Default value set.").arg(mode).arg(strBrowseName),
			QUaLogLevel::Warning,
			QUaLogCategory::Serialization
		);
	}
	// CyclicWritePeriod
	auto period = domElem.attribute("CyclicWritePeriod").toInt(&bOK);
	if (bOK)
	{
		this->setCyclicWritePeriod(period);
	}
	else
	{
		errorLogs << QUaLog(
			tr("Invalid CyclicWritePeriod attribute '%1' in Value %2. Default value set.").arg(period).arg(strBrowseName),
			QUaLogLevel::Warning,
			QUaLogCategory::Serialization
		);
	}
#endif // !QUAMODBUS_NOCYCLIC_WRITE
}

int QUaModbusValue::typeBlockSize(const QModbusValueType & type)
{
	int iSize = 0;
	switch (type)
	{
		case Binary0:
		case Binary1:
		case Binary2:
		case Binary3:
		case Binary4:
		case Binary5:
		case Binary6:
		case Binary7:
		case Binary8:
		case Binary9:
		case Binary10:
		case Binary11:
		case Binary12:
		case Binary13:
		case Binary14:
		case Binary15:
		case Decimal:
		{
			iSize = 1;
			break;
		}
		case Int:
		case IntSwapped:
		case Float:
		case FloatSwapped:
		{
			iSize = 2;
			break;
		}
		case Int64:
		case Int64Swapped:
		case Float64:
		case Float64Swapped:
		{
			iSize = 4;
			break;
		}
		default /*Invalid*/:
		{
			break;
		}
	}
	return iSize;
}

QMetaType::Type QUaModbusValue::typeToMeta(const QModbusValueType & type)
{
	QMetaType::Type metaType = QMetaType::UnknownType;
	switch (type)
	{
		case Binary0:
		case Binary1:
		case Binary2:
		case Binary3:
		case Binary4:
		case Binary5:
		case Binary6:
		case Binary7:
		case Binary8:
		case Binary9:
		case Binary10:
		case Binary11:
		case Binary12:
		case Binary13:
		case Binary14:
		case Binary15:
		{
			metaType = QMetaType::Bool;
			break;
		}
		case Decimal:
		{
			metaType = QMetaType::Short;
			break;
		}
		case Int:
		case IntSwapped:
		{
			metaType = QMetaType::Int;
			break;
		}
		case Float:
		case FloatSwapped:
		{
			metaType = QMetaType::Float;
			break;
		}
		case Int64:
		case Int64Swapped:
		{
			metaType = QMetaType::LongLong;
			break;
		}
		case Float64:
		case Float64Swapped:
		{
			metaType = QMetaType::Double;
			break;
		}
		default /*Invalid*/:
		{
			break;
		}
	}
	return metaType;
}

QVariant QUaModbusValue::blockToValue(const QVector<quint16>& block, const QModbusValueType & type)
{
	QVariant retVar;
	switch(type)
	{
		case Binary0        :
		{
			Q_ASSERT(block.count() >= 1);
			if (block.first() > 0)
			{
				retVar = QVariant::fromValue(true);
			}
			else
			{
				retVar = QVariant::fromValue(false);
			}
			break;
		}
		case Binary1        :
		case Binary2        :
		case Binary3        :
		case Binary4        :
		case Binary5        :
		case Binary6        :
		case Binary7        :
		case Binary8        :
		case Binary9        :
		case Binary10       :
		case Binary11       :
		case Binary12       :
		case Binary13       :
		case Binary14       :
		case Binary15       :
		{
			Q_ASSERT(block.count() >= 1);
			// shift uiValue bits to right 'type' times
			quint16 iTmp = block.first() >> type;
			iTmp &= 0x0001;
			if (iTmp == 1)
			{
				retVar = QVariant::fromValue(true);
			}
			else
			{
				retVar = QVariant::fromValue(false);
			}
			break;
		}
		case Decimal        :
		{
			Q_ASSERT(block.count() >= 1);
			retVar = QVariant::fromValue(block.first());
			break;
		}
		case Int            :
		{
			Q_ASSERT(block.count() >= 2);
			// i32 Least Significant Register First
			int iRes = (int)(((quint32)block.at(1) << 16) | ((quint32)block.at(0)));
			retVar = QVariant::fromValue(iRes);
			break;
		}
		case IntSwapped     :
		{
			Q_ASSERT(block.count() >= 2);
			int iRes = (int)(((quint32)block.at(0) << 16) | ((quint32)block.at(1)));
			retVar = QVariant::fromValue(iRes);
			break;
		}
		case Float          :
		{
			Q_ASSERT(block.count() >= 2);
			float fRes = 0;
			// f32 Least Significant Register First
			quint32 iTmp  = (((quint32)block.at(1) << 16) | ((quint32)block.at(0)));
			memcpy(&fRes, &iTmp, sizeof(quint32));
			retVar = QVariant::fromValue(fRes);
			break;
		}
		case FloatSwapped   :
		{
			Q_ASSERT(block.count() >= 2);
			float fRes = 0;
			// f32 Most Significant Register First
			quint32 iTmp = (((quint32)block.at(0) << 16) | ((quint32)block.at(1)));
			memcpy(&fRes, &iTmp, sizeof(quint32));
			retVar = QVariant::fromValue(fRes);
			break;
		}
		case Int64          :
		{
			Q_ASSERT(block.count() >= 4);
			qint64 iRes = 0;
			// i64 Least Significant Register First
			quint64 iTmp = (((quint64)block.at(3) << 48) | 
				            ((quint64)block.at(2) << 32) | 
				            ((quint64)block.at(1) << 16) | 
				            ((quint64)block.at(0)));
			memcpy(&iRes, &iTmp, sizeof(quint64));
			retVar = QVariant::fromValue(iRes);
			break;
		}
		case Int64Swapped   :
		{
			Q_ASSERT(block.count() >= 4);
			qint64 iRes = 0;
			// i64 Most Significant Register First
			quint64 iTmp = (((quint64)block.at(0) << 48) | 
				            ((quint64)block.at(1) << 32) | 
				            ((quint64)block.at(2) << 16) | 
				            ((quint64)block.at(3)));
			memcpy(&iRes, &iTmp, sizeof(quint64));
			retVar = QVariant::fromValue(iRes);
			break;
		}
		case Float64        :
		{
			Q_ASSERT(block.count() >= 4);
			double dRes = 0;
			// f64 Least Significant Register First
			quint64 iTmp = (((quint64)block.at(3) << 48) | 
				            ((quint64)block.at(2) << 32) | 
				            ((quint64)block.at(1) << 16) | 
				            ((quint64)block.at(0)));
			memcpy(&dRes, &iTmp, sizeof(quint64));
			retVar = QVariant::fromValue(dRes);
			break;
		}
		case Float64Swapped :
		{
			Q_ASSERT(block.count() >= 4);
			double dRes = 0;
			// f64 Most Significant Register First
			quint64 iTmp = (((quint64)block.at(0) << 48) | 
				            ((quint64)block.at(1) << 32) | 
				            ((quint64)block.at(2) << 16) | 
				            ((quint64)block.at(3)));
			memcpy(&dRes, &iTmp, sizeof(quint64));
			retVar = QVariant::fromValue(dRes);
			break;
		}
		default : // Invalid
		{
			break;
		}
	}
	return retVar;
}

QVector<quint16> QUaModbusValue::valueToBlock(const QVariant & value, const QModbusValueType & type)
{
	QVector<quint16> block;
	switch(type)
	{
		case Binary0        :
		{
			block.resize(1);
			bool bValue = value.toBool();		
			if (bValue)
			{
				block[0] = 1;
			}
			else
			{
				block[0] = 0;
			}
			break;
		}
		case Binary1        :
		case Binary2        :
		case Binary3        :
		case Binary4        :
		case Binary5        :
		case Binary6        :
		case Binary7        :
		case Binary8        :
		case Binary9        :
		case Binary10       :
		case Binary11       :
		case Binary12       :
		case Binary13       :
		case Binary14       :
		case Binary15       :
		{
			block.resize(1);
			bool bValue = value.toBool();
			if (bValue)
			{
				// shift iTmp bits to left [type] times
				quint16 iTmp = 0x0001;
				block[0] = iTmp << type;
			}
			else
			{
				block[0] = 0;
			}
			break;
		}
		case Decimal        :
		{
			block.resize(1);
			block[0] = (quint16)value.toUInt();
			break;
		}
		case Int            :
		{
			block.resize(2);
			int iValue = value.toInt();
			// i32 Least Significant Register First
			block[0] = (quint16)(iValue & 0x0000FFFFuL);
			block[1] = (quint16)(iValue >> 16);
			break;
		}
		case IntSwapped     :
		{
			block.resize(2);
			int iValue = value.toInt();
			// i32 Most Significant Register First
			block[1] = (quint16)(iValue & 0x0000FFFFuL);
			block[0] = (quint16)(iValue >> 16);
			break;
		}
		case Float          :
		{
			block.resize(2);
			float fValue = value.toFloat();
			// f32 Least Significant Register First
			quint32 iTmp;
			memcpy(&iTmp, &fValue, sizeof(quint32));
			block[0] = (quint16)(iTmp & 0x0000FFFFuL);
			block[1] = (quint16)(iTmp >> 16);
			break;
		}
		case FloatSwapped   :
		{
			block.resize(2);
			float fValue = value.toFloat();
			// f32 Most Significant Register First
			quint32 iTmp;
			memcpy(&iTmp, &fValue, sizeof(quint32));
			block[1] = (quint16)(iTmp & 0x0000FFFFuL);
			block[0] = (quint16)(iTmp >> 16);
			break;
		}
		case Int64          :
		{
			block.resize(4);
			qint64 iValue = value.toLongLong();
			// i64 Least Significant Register First
			quint64 iTmp;
			memcpy(&iTmp, &iValue, sizeof(quint64));
			block[0] = (quint16)(iTmp & 0x000000000000FFFFuLL);
			block[1] = (quint16)(iTmp >> 16);
			block[2] = (quint16)(iTmp >> 32);
			block[3] = (quint16)(iTmp >> 48);
			break;
		}
		case Int64Swapped   :
		{
			block.resize(4);
			qint64 iValue = value.toLongLong();
			// i64 Most Significant Register First
			quint64 iTmp;
			memcpy(&iTmp, &iValue, sizeof(quint64));
			block[3] = (quint16)(iTmp & 0x000000000000FFFFuLL);
			block[2] = (quint16)(iTmp >> 16);
			block[1] = (quint16)(iTmp >> 32);
			block[0] = (quint16)(iTmp >> 48);
			break;
		}
		case Float64        :
		{
			block.resize(4);
			double dValue = value.toDouble();
			// f64 Least Significant Register First
			quint64 iTmp;
			memcpy(&iTmp, &dValue, sizeof(quint64));
			block[0] = (quint16)(iTmp & 0x000000000000FFFFuLL);
			block[1] = (quint16)(iTmp >> 16);
			block[2] = (quint16)(iTmp >> 32);
			block[3] = (quint16)(iTmp >> 48);
			break;
		}
		case Float64Swapped :
		{
			block.resize(4);
			double dValue = value.toDouble();
			// f64 Most Significant Register First
			quint64 iTmp;
			memcpy(&iTmp, &dValue, sizeof(quint64));
			block[3] = (quint16)(iTmp & 0x000000000000FFFFuLL);
			block[2] = (quint16)(iTmp >> 16);
			block[1] = (quint16)(iTmp >> 32);
			block[0] = (quint16)(iTmp >> 48);
			break;
		}
		default : // Invalid
		{
			break;
		}
	}
	return block;
}

QUaModbusValueList * QUaModbusValue::list() const
{
	return qobject_cast<QUaModbusValueList*>(this->parent());
}

QUaModbusDataBlock * QUaModbusValue::block() const
{
	return this->list()->block();
}

QUaModbusClient* QUaModbusValue::client() const
{
	return this->block()->client();
}
