#include "quamodbusclient.h"

QUaModbusClient::QUaModbusClient(QUaServer *server)
	: QUaBaseObject(server)
{
	// set defaults

	type()->setValue("Tcp");

	state()->setDataTypeEnum(QMetaEnum::fromType<QModbusDevice::State>());
	state()->setValue(QModbusDevice::State::UnconnectedState);

}

QUaProperty * QUaModbusClient::type()
{
	return this->browseChild<QUaProperty>("type");
}

QUaBaseDataVariable * QUaModbusClient::state()
{
	return this->browseChild<QUaBaseDataVariable>("state");
}

QString QUaModbusClient::setType(QString strType)
{
	// check not connected
	if (state()->value().value<QModbusDevice::State>() != QModbusDevice::State::UnconnectedState)
	{
		return "Error : Cannot change Modbus client type while connected.";
	}
	// set type is valid
	strType = strType.trimmed();
	if (strType.compare("Tcp", Qt::CaseInsensitive) == 0)
	{
		type()->setValue("Tcp");
		return "Success";
	}
	else if (strType.compare("Serial", Qt::CaseInsensitive) == 0)
	{
		type()->setValue("Serial");
		return "Success";
	}
	return QString("Error : Invalid Modbus client type %1. Only \"Tcp\" and \"Serial\" types are supported.").arg(strType);
}