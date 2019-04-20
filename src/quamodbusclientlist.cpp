#include "quamodbusclientlist.h"

#include "quamodbustcpclient.h"
#include "quamodbusrtuserialclient.h"

#include "quamodbusdatablocklist.h"
#include "quamodbusdatablock.h"

#include "quamodbusvaluelist.h"
#include "quamodbusvalue.h"

#include <QUaServer>

QUaModbusClientList::QUaModbusClientList(QUaServer *server)
	: QUaFolderObject(server)
{
	// register modbus client types
	server->registerType<QUaModbusTcpClient      >();
	server->registerType<QUaModbusRtuSerialClient>();
	server->registerType<QUaModbusDataBlockList  >();
	server->registerType<QUaModbusDataBlock      >();
	server->registerType<QUaModbusValueList      >();
	server->registerType<QUaModbusValue          >();
	// register enums
	server->registerEnum<QModbusDevice::State    >();
	server->registerEnum<QModbusDevice::Error    >();
	server->registerEnum<QSerialPort::Parity     >();
	server->registerEnum<QSerialPort::BaudRate   >();
	server->registerEnum<QSerialPort::DataBits   >();
	server->registerEnum<QSerialPort::StopBits   >();
	//server->registerEnum<QUaModbusDataBlock::RegisterType>();
}

Q_INVOKABLE QString QUaModbusClientList::addTcpClient(QString strClientId)
{
	return this->addClient<QUaModbusTcpClient>(strClientId);
}

Q_INVOKABLE QString QUaModbusClientList::addRtuSerialClient(QString strClientId)
{
	return this->addClient<QUaModbusRtuSerialClient>(strClientId);
}