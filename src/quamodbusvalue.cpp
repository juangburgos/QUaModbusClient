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
	lastError    ()->setDataTypeEnum(QMetaEnum::fromType<QUaModbusValue::ValueError>());
	lastError    ()->setValue(QUaModbusValue::ValueError::NoError);
	// set initial conditions
	type         ()->setWriteAccess(true);
	addressOffset()->setWriteAccess(true);
	value        ()->setWriteAccess(false); // set to true, when type != ValueType::Invalid

	// TODO : changes

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

void QUaModbusValue::updateValue(const QVector<quint16>& block)
{
	auto type = this->type()->value().value<QUaModbusValue::ValueType>();
	if (type == QUaModbusValue::ValueType::Invalid)
	{
		return;
	}
	int addressOffset = this->addressOffset()->value().value<int>();
	if (addressOffset < 0)
	{
		return;
	}

	// TODO

}

QVariant QUaModbusValue::fromBlockToValue(const QVector<quint16>& block, const QUaModbusValue::ValueType & type)
{
	// TODO
	return QVariant();
}

QVector<quint16> QUaModbusValue::fromValueToBlock(const QVariant & value, const QUaModbusValue::ValueType & type)
{
	// TODO
	return QVector<quint16>();
}

QUaModbusDataBlock * QUaModbusValue::block()
{
	return dynamic_cast<QUaModbusValueList*>(this->parent())->block();
}
