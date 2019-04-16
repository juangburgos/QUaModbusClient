#include "quamodbusdatablock.h"
#include "quamodbusclient.h"

QUaModbusDataBlock::QUaModbusDataBlock(QUaServer *server)
	: QUaBaseObject(server)
{
	// NOTE : QObject parent might not be yet available in constructor
	modiconStartAddress()->setDataType(QMetaType::UShort);
	length()->setDataType(QMetaType::UShort);
}

QUaProperty * QUaModbusDataBlock::modiconStartAddress()
{
	return this->browseChild<QUaProperty>("ModiconStartAddress");
}

QUaProperty * QUaModbusDataBlock::length()
{
	return this->browseChild<QUaProperty>("Length");
}

void QUaModbusDataBlock::remove()
{
	this->deleteLater();
}

QUaModbusClient * QUaModbusDataBlock::client()
{
	return dynamic_cast<QUaModbusDataBlockList*>(this->parent())->client();
}
