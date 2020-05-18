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
	type         ()->setDataTypeEnum(QMetaEnum::fromType<QModbusValueType>());
	type         ()->setValue(QModbusValueType::Invalid);
	registersUsed()->setDataType(QMetaType::UShort);
	registersUsed()->setValue(0);
	addressOffset()->setDataType(QMetaType::Int);
	addressOffset()->setValue(-1);
	lastError    ()->setDataTypeEnum(QMetaEnum::fromType<QModbusError>());
	lastError    ()->setValue(QModbusError::ConfigurationError);
	// set initial conditions
	type         ()->setWriteAccess(true);
	addressOffset()->setWriteAccess(true);
	value        ()->setWriteAccess(false); // set to true, when type != ValueType::Invalid
	// handle state changes
	QObject::connect(type()         , &QUaBaseVariable::valueChanged, this, &QUaModbusValue::on_typeChanged         , Qt::QueuedConnection);
	QObject::connect(addressOffset(), &QUaBaseVariable::valueChanged, this, &QUaModbusValue::on_addressOffsetChanged, Qt::QueuedConnection);
	QObject::connect(value()        , &QUaBaseVariable::valueChanged, this, &QUaModbusValue::on_valueChanged        , Qt::QueuedConnection);
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
}

QUaProperty * QUaModbusValue::type() const
{
	return const_cast<QUaModbusValue*>(this)->browseChild<QUaProperty>("Type");
}

QUaProperty * QUaModbusValue::registersUsed() const
{
	return const_cast<QUaModbusValue*>(this)->browseChild<QUaProperty>("RegistersUsed");
}

QUaProperty * QUaModbusValue::addressOffset() const
{
	return const_cast<QUaModbusValue*>(this)->browseChild<QUaProperty>("AddressOffset");
}

QUaBaseDataVariable * QUaModbusValue::value() const
{
	return const_cast<QUaModbusValue*>(this)->browseChild<QUaBaseDataVariable>("Value");
}

QUaBaseDataVariable * QUaModbusValue::lastError() const
{
	return const_cast<QUaModbusValue*>(this)->browseChild<QUaBaseDataVariable>("LastError");
}

void QUaModbusValue::remove()
{
	this->deleteLater();
}

void QUaModbusValue::on_typeChanged(const QVariant &value)
{
	auto type = value.value<QModbusValueType>();
	// convert to metatype to set as UA type
	auto metaType = QUaModbusValue::typeToMeta(type);
	this->value()->setDataType(metaType);
	// update value is possible
	auto blockVariant = this->block()->data()->value();
	auto blockError   = this->block()->lastError()->value().value<QModbusError>();
	auto blockData    = QUaModbusDataBlock::variantToInt16Vect(blockVariant);
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
	return this->type()->value().value<QModbusValueType>();
}

void QUaModbusValue::setType(const QModbusValueType & type)
{
	if (this->getType() == type)
	{
		return;
	}
	this->type()->setValue(type);
	this->on_typeChanged(type);
}

quint16 QUaModbusValue::getRegistersUsed() const
{
	return this->registersUsed()->value().value<quint16>();
}

int QUaModbusValue::getAddressOffset() const
{
	return this->addressOffset()->value().toInt();
}

void QUaModbusValue::setAddressOffset(const int & addressOffset)
{
	this->addressOffset()->setValue(addressOffset);
	this->on_addressOffsetChanged(addressOffset);
}

QVariant QUaModbusValue::getValue() const
{
	return this->value()->value();
}

// programmatic change from C++ API
void QUaModbusValue::setValue(const QVariant & value)
{
	this->value()->setValue(value);
	this->on_valueChanged(value, true);
}

QModbusError QUaModbusValue::getLastError() const
{
	return this->lastError()->value().value<QModbusError>();
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

void QUaModbusValue::on_addressOffsetChanged(const QVariant & value)
{
	// update value is possible
	auto blockVariant = this->block()->data()->value();
	auto blockError   = this->block()->lastError()->value().value<QModbusError>();
	auto blockData    = QUaModbusDataBlock::variantToInt16Vect(blockVariant);
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
	auto type = this->type()->value().value<QModbusValueType>();
	auto partBlockData = QUaModbusValue::valueToBlock(value, type);
	// get current block
	auto blockVariant = this->block()->data()->value();
	auto blockError   = this->block()->lastError()->value().value<QModbusError>();
	auto blockData    = QUaModbusDataBlock::variantToInt16Vect(blockVariant);
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
	int addressOffset = this->addressOffset()->value().value<int>();
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
	if (addressOffset + typeBlockSize > blockData.count())
	{
		this->value()->setWriteAccess(false);
		this->value()->setValue(QVariant()); // NOTE : avoid recursion
		this->setLastError(QModbusError::ConfigurationError);
		// emit
		emit this->valueChanged(QVariant());
		return;
	}
	// replace part
	for (int i = 0; i < partBlockData.count(); i++)
	{
		blockData.replace(addressOffset + i, partBlockData.at(i));
	}
	// update block (i.e. update OPC UA)
	this->block()->on_dataChanged(QVariant::fromValue(blockData));
	// emit
	emit this->valueChanged(value);
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
	// return value element
	return elemValue;
}

void QUaModbusValue::fromDomElement(QDomElement & domElem, QString & strError)
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
	auto type = (QModbusValueType)QMetaEnum::fromType<QModbusValueType>().keysToValue(domElem.attribute("Type").toUtf8(), &bOK);
	if (bOK)
	{
		this->setType(type);
	}
	else
	{
		strError += tr("%1 : Invalid Type attribute '%2' in Value %3. Default value set.\n").arg("Warning").arg(type).arg(strBrowseName);
	}
	// AddressOffset
	auto addressOffset = domElem.attribute("AddressOffset").toInt(&bOK);
	if (bOK)
	{
		this->setAddressOffset(addressOffset);
	}
	else
	{
		strError += tr("%1 : Invalid AddressOffset attribute '%1' in Value %2. Default value set.\n").arg("Warning").arg(addressOffset).arg(strBrowseName);
	}
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
