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
	auto blockError   = this->block()->lastError()->value().value<QModbusDevice::Error>();
	auto blockData    = QUaModbusDataBlock::variantToInt16Vect(blockVariant);
	this->setValue(blockData, blockError);
}

void QUaModbusValue::on_addressOffsetChanged(const QVariant & value)
{
	Q_UNUSED(value);
	// update value is possible
	auto blockVariant = this->block()->data()->value();
	auto blockError   = this->block()->lastError()->value().value<QModbusDevice::Error>();
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
	if (addressOffset + typeBlockSize > blockData.count())
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
	if (addressOffset + typeBlockSize > block.count())
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
			retVar = QVariant::fromValue((int)block.first());
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

QVector<quint16> QUaModbusValue::valueToBlock(const QVariant & value, const QUaModbusValue::ValueType & type)
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

QUaModbusDataBlock * QUaModbusValue::block()
{
	return dynamic_cast<QUaModbusValueList*>(this->parent())->block();
}
