#include "quamodbusvalue.h"
#include "quamodbusvaluelist.h"
#include "quamodbusdatablock.h"

#include <QUaProperty>
#include <QUaBaseDataVariable>

QUaModbusValue::QUaModbusValue(QUaServer *server)
	: QUaBaseObject(server)
{
	// set defaults
	type         ()->setDataTypeEnum(QMetaEnum::fromType<QUaModbusValue::ValueType>());
	type         ()->setValue(QUaModbusValue::ValueType::Invalid);
	addressOffset()->setDataType(QMetaType::Int);
	addressOffset()->setValue(-1);
	lastError    ()->setDataTypeEnum(QMetaEnum::fromType<QModbusDevice::Error>());
	lastError    ()->setValue(QModbusDevice::Error::ConfigurationError);
	// set initial conditions
	type         ()->setWriteAccess(true);
	addressOffset()->setWriteAccess(true);
	value        ()->setWriteAccess(false); // set to true, when type != ValueType::Invalid
	// handle state changes
	QObject::connect(type()         , &QUaBaseVariable::valueChanged, this, &QUaModbusValue::on_typeChanged         , Qt::QueuedConnection);
	QObject::connect(addressOffset(), &QUaBaseVariable::valueChanged, this, &QUaModbusValue::on_addressOffsetChanged, Qt::QueuedConnection);
	QObject::connect(value()        , &QUaBaseVariable::valueChanged, this, &QUaModbusValue::on_valueChanged        , Qt::QueuedConnection);
}

QUaProperty * QUaModbusValue::type()
{
	return this->browseChild<QUaProperty>("Type");
}

QUaProperty * QUaModbusValue::addressOffset()
{
	return this->browseChild<QUaProperty>("AddressOffset");
}

QUaBaseDataVariable * QUaModbusValue::value()
{
	return this->browseChild<QUaBaseDataVariable>("Value");
}

QUaBaseDataVariable * QUaModbusValue::lastError()
{
	return this->browseChild<QUaBaseDataVariable>("LastError");
}

void QUaModbusValue::remove()
{
	this->deleteLater();
}

void QUaModbusValue::on_typeChanged(const QVariant &value)
{
	auto type = value.value<QUaModbusValue::ValueType>();
	// convert to metatype to set as UA type
	auto metaType = QUaModbusValue::typeToMeta(type);
	this->value()->setDataType(metaType);
	// update value is possible
	auto blockVariant = this->block()->data()->value();
	auto blockError   = this->block()->lastError()->value().value< QModbusDevice::Error>();
	auto blockData    = QUaModbusDataBlock::variantToInt16Vect(blockVariant);
	this->setValue(blockData, blockError);
}

void QUaModbusValue::on_addressOffsetChanged(const QVariant & value)
{
	Q_UNUSED(value);
	// update value is possible
	auto blockVariant = this->block()->data()->value();
	auto blockError   = this->block()->lastError()->value().value< QModbusDevice::Error>();
	auto blockData    = QUaModbusDataBlock::variantToInt16Vect(blockVariant);
	this->setValue(blockData, blockError);
}

// network change
void QUaModbusValue::on_valueChanged(const QVariant & value)
{
	// get block representation of value
	auto type = this->type()->value().value<QUaModbusValue::ValueType>();
	auto partBlockData = QUaModbusValue::valueToBlock(value, type);
	// get current block
	auto blockVariant = this->block()->data()->value();
	auto blockData    = QUaModbusDataBlock::variantToInt16Vect(blockVariant);
	// check if fits in block
	int addressOffset = this->addressOffset()->value().value<int>();
	int typeBlockSize = QUaModbusValue::typeBlockSize(type);
	if (addressOffset + typeBlockSize < blockData.count())
	{
		this->value()->setWriteAccess(false);
		this->value()->setValue(QVariant());
		lastError()->setValue(QModbusDevice::Error::ConfigurationError);
		return;
	}
	// replace part
	for (int i = 0; i < partBlockData.count(); i++)
	{
		blockData.replace(addressOffset + i, partBlockData.at(i));
	}
	// update block
	this->block()->on_dataChanged(QVariant::fromValue(blockData));
}

// programmatic change
void QUaModbusValue::setValue(const QVector<quint16>& block, const QModbusDevice::Error &blockError)
{
	// check configuration
	auto type = this->type()->value().value<QUaModbusValue::ValueType>();
	if (type == QUaModbusValue::ValueType::Invalid)
	{
		this->value()->setWriteAccess(false);
		this->value()->setValue(QVariant());
		lastError()->setValue(QModbusDevice::Error::ConfigurationError);
		return;
	}
	int addressOffset = this->addressOffset()->value().value<int>();
	if (addressOffset < 0)
	{
		this->value()->setWriteAccess(false);
		this->value()->setValue(QVariant());
		lastError()->setValue(QModbusDevice::Error::ConfigurationError);
		return;
	}
	// check if fits in block
	int typeBlockSize = QUaModbusValue::typeBlockSize(type);
	if (addressOffset + typeBlockSize < block.count())
	{
		this->value()->setWriteAccess(false);
		this->value()->setValue(QVariant());
		lastError()->setValue(QModbusDevice::Error::ConfigurationError);
		return;
	}
	// convert value and set it, but leave block error code
	this->value()->setWriteAccess(true);
	lastError()->setValue(blockError);
	auto value = QUaModbusValue::blockToValue(block.mid(addressOffset, typeBlockSize), type);
	this->value()->setValue(value);
}

void QUaModbusValue::setError(const QModbusDevice::Error & blockError)
{
	lastError()->setValue(blockError);
}

int QUaModbusValue::typeBlockSize(const QUaModbusValue::ValueType & type)
{
	int iSize = -1;
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

QMetaType::Type QUaModbusValue::typeToMeta(const QUaModbusValue::ValueType & type)
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

QVariant QUaModbusValue::blockToValue(const QVector<quint16>& block, const QUaModbusValue::ValueType & type)
{
	// TODO
/*
	switch(type)
	{
		case Binary0        :
		{

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

			break;
		}
		case Decimal        :
		{

			break;
		}
		case Int            :
		{

			break;
		}
		case IntSwapped     :
		{

			break;
		}
		case Float          :
		{

			break;
		}
		case FloatSwapped   :
		{

			break;
		}
		case Int64          :
		{

			break;
		}
		case Int64Swapped   :
		{

			break;
		}
		case Float64        :
		{

			break;
		}
		case Float64Swapped :
		{

			break;
		}
		default : // Invalid
		{

			break;
		}
	}
*/

	return QVariant();
}

QVector<quint16> QUaModbusValue::valueToBlock(const QVariant & value, const QUaModbusValue::ValueType & type)
{
	// TODO
	return QVector<quint16>();
}

QUaModbusDataBlock * QUaModbusValue::block()
{
	return dynamic_cast<QUaModbusValueList*>(this->parent())->block();
}
