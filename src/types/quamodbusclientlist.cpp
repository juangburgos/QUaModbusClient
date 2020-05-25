#include "quamodbusclientlist.h"

#include "quamodbustcpclient.h"
#include "quamodbusrtuserialclient.h"

#include "quamodbusdatablocklist.h"
#include "quamodbusdatablock.h"

#include "quamodbusvaluelist.h"
#include "quamodbusvalue.h"

#include <QUaServer>

#ifdef QUA_ACCESS_CONTROL
#include <QUaPermissions>
#include <QUaPermissionsList>
#endif // QUA_ACCESS_CONTROL

QUaModbusClientList::QUaModbusClientList(QUaServer *server)
#ifndef QUA_ACCESS_CONTROL
	: QUaFolderObject(server)
#else
	: QUaFolderObjectProtected(server)
#endif // !QUA_ACCESS_CONTROL
{
	// register custom types (also registers enums of custom types)
	server->registerType<QUaModbusTcpClient      >();
	server->registerType<QUaModbusRtuSerialClient>();
	server->registerType<QUaModbusDataBlockList  >();
	server->registerType<QUaModbusDataBlock      >();
	server->registerType<QUaModbusValueList      >();
	server->registerType<QUaModbusValue          >();
	// register enums (need to register enums that are not part of custom types)
	server->registerEnum<QModbusClientType>();
	server->registerEnum<QModbusState     >();
	server->registerEnum<QModbusError     >();
	server->registerEnum<QParity          >();
	server->registerEnum<QBaudRate        >();
	server->registerEnum<QDataBits        >();
	server->registerEnum<QStopBits        >();
	server->registerEnum(QUaModbusRtuSerialClient::ComPorts, QUaModbusRtuSerialClient::EnumComPorts());
}

QString QUaModbusClientList::addTcpClient(const QUaQualifiedName& clientId)
{
	return this->addClient<QUaModbusTcpClient>(clientId);
}

QString QUaModbusClientList::addRtuSerialClient(const QUaQualifiedName& clientId)
{
	return this->addClient<QUaModbusRtuSerialClient>(clientId);
}

void QUaModbusClientList::clear()
{
	for (auto client : this->clients())
	{
		client->remove();
	}
}

QString QUaModbusClientList::xmlConfig()
{
	QDomDocument doc;
	// set xml header
	QDomProcessingInstruction header = doc.createProcessingInstruction("xml", "version='1.0' encoding='UTF-8'");
	doc.appendChild(header);
	// convert config to xml
	auto elemClients = this->toDomElement(doc);
	doc.appendChild(elemClients);
	// get config
	return doc.toByteArray();
}

QString QUaModbusClientList::setXmlConfig(QString strXmlConfig)
{
	QQueue<QUaLog> errorLogs;
	// set to dom doc
	QDomDocument doc;
	int line, col;
	QString strError;
	doc.setContent(strXmlConfig, &strError, &line, &col);
	if (!strError.isEmpty())
	{
		errorLogs << QUaLog(
			tr("Invalid XML in Line %1 Column %2 Error %3").arg(line).arg(col).arg(strError),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		);
		return QUaLog::toString(errorLogs);
	}
	// get list of clients
	QDomElement elemClientList = doc.firstChildElement(QUaModbusClientList::staticMetaObject.className());
	if (elemClientList.isNull())
	{
		errorLogs << QUaLog(
			tr("No Modbus client list found in XML config."),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		);
		return QUaLog::toString(errorLogs);
	}
	// load config from xml
	this->fromDomElement(elemClientList, errorLogs);
	if (!errorLogs.isEmpty())
	{
		QUaLog::toString(errorLogs);
	}
	return "Success.";
}

QString QUaModbusClientList::csvClients()
{
	QString strCsv;
#ifndef QUA_ACCESS_CONTROL
	strCsv += QString("%1, %2, %3, %4, %5, %6\n")
#else
	strCsv += QString("%1, %2, %3, %4, %5, %6, %7\n")
#endif // !QUA_ACCESS_CONTROL
		.arg(tr("Name"          ))
		.arg(tr("Type"          ))
		.arg(tr("ServerAddress" ))
		.arg(tr("KeepConnecting"))
		.arg(tr("NetworkAddress"))
		.arg(tr("NetworkPort"   ))
#ifndef QUA_ACCESS_CONTROL
		;
#else
		.arg(tr("Permissions"));
#endif // !QUA_ACCESS_CONTROL
		
	auto clientsTcp = this->browseChildren<QUaModbusTcpClient>();
	for (auto clientTcp : clientsTcp)
	{
		auto strType = QString(QMetaEnum::fromType<QModbusClientType>().valueToKey(clientTcp->getType()));
#ifndef QUA_ACCESS_CONTROL
		strCsv += QString("%1, %2, %3, %4, %5, %6\n")
#else
		strCsv += QString("%1, %2, %3, %4, %5, %6, %7\n")
#endif // !QUA_ACCESS_CONTROL
			.arg(clientTcp->browseName().name())
			.arg(strType)
			.arg(clientTcp->getServerAddress ())
			.arg(clientTcp->getKeepConnecting())
			.arg(clientTcp->getNetworkAddress())
			.arg(clientTcp->getNetworkPort   ())
#ifndef QUA_ACCESS_CONTROL
			;
#else
			.arg(clientTcp->permissionsObject() ? clientTcp->permissionsObject()->browseName().name() : "");
#endif // !QUA_ACCESS_CONTROL	
	}
#ifndef QUA_ACCESS_CONTROL
	strCsv += QString("%1, %2, %3, %4, %5, %6, %7, %8, %9\n")
#else
	strCsv += QString("%1, %2, %3, %4, %5, %6, %7, %8, %9, %10\n")
#endif // !QUA_ACCESS_CONTROL
	
		.arg(tr("Name"          ))
		.arg(tr("Type"          ))
		.arg(tr("ServerAddress" ))
		.arg(tr("KeepConnecting"))
		.arg(tr("ComPort"       ))
		.arg(tr("Parity"        ))
		.arg(tr("BaudRate"      ))
		.arg(tr("DataBits"      ))
		.arg(tr("StopBits"      ))
#ifndef QUA_ACCESS_CONTROL
		;
#else
		.arg(tr("Permissions"));
#endif // !QUA_ACCESS_CONTROL
	auto clientsSerial = this->browseChildren<QUaModbusRtuSerialClient>();
	for (auto clientSerial : clientsSerial)
	{
		auto strType     = QString(QMetaEnum::fromType<QModbusClientType>().valueToKey(clientSerial->getType()));
		auto strParity   = QString(QMetaEnum::fromType<QParity  >().valueToKey(clientSerial->getParity  ()));
		auto strBaudRate = QString(QMetaEnum::fromType<QBaudRate>().valueToKey(clientSerial->getBaudRate()));
		auto strDataBits = QString(QMetaEnum::fromType<QDataBits>().valueToKey(clientSerial->getDataBits()));
		auto strStopBits = QString(QMetaEnum::fromType<QStopBits>().valueToKey(clientSerial->getStopBits()));
#ifndef QUA_ACCESS_CONTROL
		strCsv += QString("%1, %2, %3, %4, %5, %6, %7, %8, %9\n")
#else
		strCsv += QString("%1, %2, %3, %4, %5, %6, %7, %8, %9, %10\n")
#endif // !QUA_ACCESS_CONTROL	
			.arg(clientSerial->browseName().name())
			.arg(strType)
			.arg(clientSerial->getServerAddress ())
			.arg(clientSerial->getKeepConnecting())
			.arg(clientSerial->getComPort       ())
			.arg(strParity)
			.arg(strBaudRate)
			.arg(strDataBits)
			.arg(strStopBits)
#ifndef QUA_ACCESS_CONTROL
			;
#else
			.arg(clientSerial->permissionsObject() ? clientSerial->permissionsObject()->browseName().name() : "");
#endif // !QUA_ACCESS_CONTROL	
	}
	return strCsv;
}

QString QUaModbusClientList::csvBlocks()
{
	QString strCsv;
#ifndef QUA_ACCESS_CONTROL
	strCsv += QString("%1, %2, %3, %4, %5, %6\n")
#else
	strCsv += QString("%1, %2, %3, %4, %5, %6, %7\n")
#endif // !QUA_ACCESS_CONTROL
		.arg(tr("Name"        ))
		.arg(tr("Client"      ))
		.arg(tr("Type"        ))
		.arg(tr("Address"     ))
		.arg(tr("Size"        ))
		.arg(tr("SamplingTime"))
#ifndef QUA_ACCESS_CONTROL
		;
#else
		.arg(tr("Permissions"));
#endif // !QUA_ACCESS_CONTROL
	auto clients = this->browseChildren<QUaModbusClient>();
	for (auto client : clients)
	{
		auto blocks = client->dataBlocks()->blocks();
		for (auto block : blocks)
		{
			auto strType = QString(QMetaEnum::fromType<QModbusDataBlockType>().valueToKey(block->getType()));
#ifndef QUA_ACCESS_CONTROL
			strCsv += QString("%1, %2, %3, %4, %5, %6\n")
#else
			strCsv += QString("%1, %2, %3, %4, %5, %6, %7\n")
#endif // !QUA_ACCESS_CONTROL
				.arg(block->browseName().name())
				.arg(client->browseName().name())
				.arg(strType)
				.arg(block->getAddress())
				.arg(block->getSize())
				.arg(block->getSamplingTime())
#ifndef QUA_ACCESS_CONTROL
				;
#else
				.arg(block->permissionsObject() ? block->permissionsObject()->browseName().name() : "");
#endif // !QUA_ACCESS_CONTROL
		}
	}
	return strCsv;
}

QString QUaModbusClientList::csvValues()
{
	QString strCsv;
#ifndef QUA_ACCESS_CONTROL
	strCsv += QString("%1, %2, %3, %4, %5\n")
#else
	strCsv += QString("%1, %2, %3, %4, %5, %6\n")
#endif // !QUA_ACCESS_CONTROL
		.arg(tr("Name"))
		.arg(tr("Client"))
		.arg(tr("Block"))
		.arg(tr("Type"))
		.arg(tr("AddressOffset"))
#ifndef QUA_ACCESS_CONTROL
		;
#else
		.arg(tr("Permissions"));
#endif // !QUA_ACCESS_CONTROL
	auto clients = this->browseChildren<QUaModbusClient>();
	for (auto client : clients)
	{
		auto blocks = client->dataBlocks()->blocks();
		for (auto block : blocks)
		{
			auto values = block->values()->values();
			for (auto value : values)
			{
				auto strType = QString(QMetaEnum::fromType<QModbusValueType>().valueToKey(value->getType()));
#ifndef QUA_ACCESS_CONTROL
				strCsv += QString("%1, %2, %3, %4, %5\n")
#else
				strCsv += QString("%1, %2, %3, %4, %5, %6\n")
#endif // !QUA_ACCESS_CONTROL
					.arg(value->browseName().name())
					.arg(client->browseName().name())
					.arg(block->browseName().name())
					.arg(strType)
					.arg(value->getAddressOffset())
#ifndef QUA_ACCESS_CONTROL
					;
#else
					.arg(value->permissionsObject() ? value->permissionsObject()->browseName().name() : "");
#endif // !QUA_ACCESS_CONTROL
			}
		}
	}
	return strCsv;
}

QQueue<QUaLog> QUaModbusClientList::setCsvClients(QString strCsvClients)
{
	QQueue<QUaLog> errorLogs;
	auto listRows = strCsvClients.split("\n");
	for (auto strRow : listRows)
	{
		bool bOK = false;
		auto listCols = strRow.split(",");
		// check length
		if (listCols.count() <= 1)
		{
			continue;
		}
#ifndef QUA_ACCESS_CONTROL
		if (listCols.count() < 6)
#else
		if (listCols.count() < 7)
#endif // !QUA_ACCESS_CONTROL
		{
			errorLogs << QUaLog(
				tr("Invalid column count in row [%1]. Ignoring.").arg(strRow),
				QUaLogLevel::Warning,
				QUaLogCategory::Serialization
			);
			continue;
		}
		// get name
		auto strBrowseName = listCols.at(0).trimmed();
		// ignore row if first col is "Name"
		if (strBrowseName.compare("Name", Qt::CaseSensitive) == 0)
		{
			continue;
		}
		// get type
		auto strType = listCols.at(1).trimmed();
		auto cliType = (QModbusClientType)QMetaEnum::fromType<QModbusClientType>().keysToValue(strType.toUtf8(), &bOK);
		if (!bOK)
		{
			errorLogs << QUaLog(
				tr("Invalid client type '%1' in row [%2].").arg(strType).arg(strRow),
				QUaLogLevel::Error,
				QUaLogCategory::Serialization
			);
			continue;
		}
		// get modbus address
		auto serverAddress = listCols.at(2).trimmed().toUInt(&bOK);
		if (!bOK)
		{
			errorLogs << QUaLog(
				tr("Invalid ServerAddress '%1' in row [%2]. Default value set.").arg(listCols.at(2).trimmed()).arg(strRow),
				QUaLogLevel::Warning,
				QUaLogCategory::Serialization
			);
		}
		// get keep connecting
		auto keepConnecting = (bool)listCols.at(3).trimmed().toUInt(&bOK);
		if (!bOK)
		{
			errorLogs << QUaLog(
				tr("Invalid KeepConnecting '%1' in row [%2]. Default value set.").arg(listCols.at(3).trimmed()).arg(strRow),
				QUaLogLevel::Warning,
				QUaLogCategory::Serialization
			);
		}
		// type dependent
		switch (cliType)
		{
		case QModbusClientType::Tcp:
			{
				// get network address
				auto networkAddress = listCols.at(4).trimmed();
				if (networkAddress.isEmpty())
				{
					errorLogs << QUaLog(
						tr("Invalid NetworkAddress '%1' in row [%2]. Default value set.").arg(listCols.at(4).trimmed()).arg(strRow),
						QUaLogLevel::Warning,
						QUaLogCategory::Serialization
					);
				}
				// get network port
				auto networkPort = listCols.at(5).trimmed().toUInt(&bOK);
				if (!bOK)
				{
					errorLogs << QUaLog(
						tr("Invalid NetworkPort '%1' in row [%2]. Default value set.").arg(listCols.at(5).trimmed()).arg(strRow),
						QUaLogLevel::Warning,
						QUaLogCategory::Serialization
					);
				}
				// check if tcp client exists
				auto clientTcp = this->browseChild<QUaModbusTcpClient>(strBrowseName);
				if (clientTcp)
				{
					errorLogs << QUaLog(
						tr("Client '%1' already exists in row [%2]. Overwriting properties.").arg(strBrowseName).arg(strRow),
						QUaLogLevel::Warning,
						QUaLogCategory::Serialization
					);
				}
				else
				{
					// check if serial client exists
					auto clientSerial = this->browseChild<QUaModbusRtuSerialClient>(strBrowseName);
					if (clientSerial)
					{
						errorLogs << QUaLog(
							tr("There already exists a serial client '%1' defined in row [%2]. Delete it first.").arg(strBrowseName).arg(strRow),
							QUaLogLevel::Error,
							QUaLogCategory::Serialization
						);
						continue;
					}
					// actually add client
					auto strNewError = this->addTcpClient(strBrowseName);
					if (strNewError.contains("Error", Qt::CaseInsensitive))
					{
						errorLogs << QUaLog(
							strNewError,
							QUaLogLevel::Error,
							QUaLogCategory::Serialization
						);
						continue;
					}
					clientTcp = this->browseChild<QUaModbusTcpClient>(strBrowseName);
					if (!clientTcp)
					{
						errorLogs << QUaLog(
							tr("Failed to find '%1' client in client list after adding row [%2].").arg(strBrowseName).arg(strRow),
							QUaLogLevel::Error,
							QUaLogCategory::Serialization
						);
						continue;
					}
				}
				// set props
				clientTcp->setServerAddress (serverAddress );
				clientTcp->setKeepConnecting(keepConnecting);
				clientTcp->setNetworkAddress(networkAddress);
				clientTcp->setNetworkPort   (networkPort   );
#ifdef QUA_ACCESS_CONTROL
				// permissions are optional (can be empty)
				auto permsBrowseName = listCols.at(6).trimmed();
				if (!permsBrowseName.isEmpty())
				{
					auto permsList = this->getPermissionsList();
					if (!permsList) { continue; }
					auto perms = permsList->permission(permsBrowseName);
					if (perms)
					{
						clientTcp->setPermissionsObject(perms);
					}
					else
					{
						errorLogs << QUaLog(
							tr("Failed to find '%1' permissions object after adding row [%2].").arg(permsBrowseName).arg(strRow),
							QUaLogLevel::Error,
							QUaLogCategory::Serialization
						);
					}
				}
#endif // QUA_ACCESS_CONTROL
			}
			break;
		case QModbusClientType::Serial:
			{
				// check length
#ifndef QUA_ACCESS_CONTROL
				if (listCols.count() < 9)
#else
				if (listCols.count() < 10)
#endif // !QUA_ACCESS_CONTROL
				{
					errorLogs << QUaLog(
						tr("Invalid column count in row [%1].").arg(strRow),
						QUaLogLevel::Error,
						QUaLogCategory::Serialization
					);
					continue;
				}
				// get com port
				auto comPort = listCols.at(4).trimmed();
				if (comPort.isEmpty())
				{
					errorLogs << QUaLog(
						tr("Invalid ComPort '%1' in row [%2]. Default value set.").arg(comPort).arg(strRow),
						QUaLogLevel::Warning,
						QUaLogCategory::Serialization
					);
				}
				// get parity
				auto strParity = listCols.at(5).trimmed();
				auto parity = (QParity)QMetaEnum::fromType<QParity>().keysToValue(strParity.toUtf8(), &bOK);
				if (!bOK)
				{
					errorLogs << QUaLog(
						tr("Invalid Parity '%1' in row [%2]. Default value set.").arg(strParity).arg(strRow),
						QUaLogLevel::Warning,
						QUaLogCategory::Serialization
					);
				}
				// get baud rate
				auto strBaudRate = listCols.at(6).trimmed();
				auto baudRate = (QBaudRate)QMetaEnum::fromType<QBaudRate>().keysToValue(strBaudRate.toUtf8(), &bOK);
				if (!bOK)
				{
					errorLogs << QUaLog(
						tr("Invalid BaudRate '%1' in row [%2]. Default value set.").arg(strBaudRate).arg(strRow),
						QUaLogLevel::Warning,
						QUaLogCategory::Serialization
					);
				}
				// get data bits
				auto strDataBits = listCols.at(7).trimmed();
				auto dataBits = (QDataBits)QMetaEnum::fromType<QDataBits>().keysToValue(strDataBits.toUtf8(), &bOK);
				if (!bOK)
				{
					errorLogs << QUaLog(
						tr("Invalid DataBits '%1' in row [%2]. Default value set.").arg(strDataBits).arg(strRow),
						QUaLogLevel::Warning,
						QUaLogCategory::Serialization
					);
				}
				// get stop bits
				auto strStopBits = listCols.at(8).trimmed();
				auto stopBits = (QStopBits)QMetaEnum::fromType<QStopBits>().keysToValue(strStopBits.toUtf8(), &bOK);
				if (!bOK)
				{
					errorLogs << QUaLog(
						tr("Invalid StopBits '%1' in row [%2]. Default value set.").arg(strStopBits).arg(strRow),
						QUaLogLevel::Warning,
						QUaLogCategory::Serialization
					);
				}
				// check if serial client exists
				auto clientSerial = this->browseChild<QUaModbusRtuSerialClient>(strBrowseName);
				if (clientSerial)
				{
					errorLogs << QUaLog(
						tr("Client '%1' already exists in row [%2]. Overwriting properties.").arg(strBrowseName).arg(strRow),
						QUaLogLevel::Warning,
						QUaLogCategory::Serialization
					);
				}
				else
				{
					// check if tcp client exists
					auto clientTcp = this->browseChild<QUaModbusTcpClient>(strBrowseName);
					if (clientTcp)
					{
						errorLogs << QUaLog(
							tr("There already exists a tcp client '%1' defined in row [%2]. Delete it first.").arg(strBrowseName).arg(strRow),
							QUaLogLevel::Error,
							QUaLogCategory::Serialization
						);
						continue;
					}
					// actually add client
					auto strNewError = this->addRtuSerialClient(strBrowseName);
					if (strNewError.contains("Error", Qt::CaseInsensitive))
					{
						errorLogs << QUaLog(
							strNewError,
							QUaLogLevel::Error,
							QUaLogCategory::Serialization
						);
						continue;
					}
					clientSerial = this->browseChild<QUaModbusRtuSerialClient>(strBrowseName);
					if (!clientSerial)
					{
						errorLogs << QUaLog(
							tr("Failed to find '%1' client in client list after adding row [%2].").arg(strBrowseName).arg(strRow),
							QUaLogLevel::Error,
							QUaLogCategory::Serialization
						);
						continue;
					}
				}
				// set props
				clientSerial->setServerAddress(serverAddress);
				clientSerial->setKeepConnecting(keepConnecting);
				clientSerial->setComPort(comPort);
				clientSerial->setParity(parity);
				clientSerial->setBaudRate(baudRate);
				clientSerial->setDataBits(dataBits);
				clientSerial->setStopBits(stopBits);
#ifdef QUA_ACCESS_CONTROL
				// permissions are optional (can be empty)
				auto permsBrowseName = listCols.at(9).trimmed();
				if (!permsBrowseName.isEmpty())
				{
					auto permsList = this->getPermissionsList();
					if (!permsList) { continue; }
					auto perms = permsList->permission(permsBrowseName);
					if (perms)
					{
						clientSerial->setPermissionsObject(perms);
					}
					else
					{
						errorLogs << QUaLog(
							tr("Failed to find '%1' permissions object after adding row [%2].").arg(permsBrowseName).arg(strRow),
							QUaLogLevel::Error,
							QUaLogCategory::Serialization
						);
					}
				}
#endif // QUA_ACCESS_CONTROL
			}
			break;
		default:
			Q_ASSERT(false);
			break;
		}
	}
	return errorLogs;
}

QQueue<QUaLog> QUaModbusClientList::setCsvBlocks(QString strCsvBlocks)
{
	QQueue<QUaLog> errorLogs;
	auto listRows = strCsvBlocks.split("\n");
	for (auto strRow : listRows)
	{
		bool bOK = false;
		auto listCols = strRow.split(",");
		// check length
		if (listCols.count() <= 1)
		{
			continue;
		}
#ifndef QUA_ACCESS_CONTROL
		if (listCols.count() < 6)
#else
		if (listCols.count() < 7)
#endif // !QUA_ACCESS_CONTROL
		{
			errorLogs << QUaLog(
				tr("Invalid column count in row [%1]. Ignoring.").arg(strRow),
				QUaLogLevel::Warning,
				QUaLogCategory::Serialization
			);
			continue;
		}
		// get name
		auto strBrowseName = listCols.at(0).trimmed();
		// ignore row if first col is "Name"
		if (strBrowseName.compare("Name", Qt::CaseSensitive) == 0)
		{
			continue;
		}
		// get client
		auto strClientName = listCols.at(1).trimmed();
		auto client = this->browseChild<QUaModbusClient>(strClientName);
		if (!client)
		{
			errorLogs << QUaLog(
				tr("There is no client '%1' defined in row [%2].").arg(strClientName).arg(strRow),
				QUaLogLevel::Error,
				QUaLogCategory::Serialization
			);
			continue;
		}
		// get type
		auto type = (QModbusDataBlockType)QMetaEnum::fromType<QModbusDataBlockType>().keysToValue(listCols.at(2).trimmed().toUtf8(), &bOK);
		if (!bOK)
		{
			errorLogs << QUaLog(
				tr("Invalid Type '%1' in row [%2]. Default value set.").arg(listCols.at(2).trimmed()).arg(strRow),
				QUaLogLevel::Warning,
				QUaLogCategory::Serialization
			);
		}
		// get address
		auto address = listCols.at(3).trimmed().toInt(&bOK);
		if (!bOK)
		{
			errorLogs << QUaLog(
				tr("Invalid Address '%1' in row [%2]. Default value set.").arg(listCols.at(3).trimmed()).arg(strRow),
				QUaLogLevel::Warning,
				QUaLogCategory::Serialization
			);
		}
		// get size
		auto size = listCols.at(4).trimmed().toUInt(&bOK);
		if (!bOK)
		{
			errorLogs << QUaLog(
				tr("Invalid Size '%1' in row [%2]. Default value set.").arg(listCols.at(4).trimmed()).arg(strRow),
				QUaLogLevel::Warning,
				QUaLogCategory::Serialization
			);
		}
		// get sampling time
		auto samplingTime = listCols.at(5).trimmed().toUInt(&bOK);
		if (!bOK)
		{
			errorLogs << QUaLog(
				tr("Invalid SamplingTime '%1' in row [%2]. Default value set.").arg(listCols.at(5).trimmed()).arg(strRow),
				QUaLogLevel::Warning,
				QUaLogCategory::Serialization
			);
		}
		// check if block exists
		auto blocks = client->dataBlocks();
		auto block  = blocks->browseChild<QUaModbusDataBlock>(strBrowseName);
		if (block)
		{
			errorLogs << QUaLog(
				tr("There already exists a block '%1.%2' defined in row [%3]. Overwriting properties.").arg(client->browseName().name()).arg(strBrowseName).arg(strRow),
				QUaLogLevel::Warning,
				QUaLogCategory::Serialization
			);
		}
		else
		{
			// actually add block
			auto strNewError = blocks->addDataBlock(strBrowseName);
			if (strNewError.contains("Error", Qt::CaseInsensitive))
			{
				errorLogs << QUaLog(
					strNewError,
					QUaLogLevel::Error,
					QUaLogCategory::Serialization
				);
				continue;
			}
			block = blocks->browseChild<QUaModbusDataBlock>(strBrowseName);
			if (!block)
			{
				errorLogs << QUaLog(
					tr("Failed to find '%1' block in client '%2' list after adding row [%3].").arg(client->browseName().name()).arg(strBrowseName).arg(strRow),
					QUaLogLevel::Error,
					QUaLogCategory::Serialization
				);
				continue;
			}
		}	
		// set properties
		block->setType(type);
		block->setAddress(address);
		block->setSize(size);
		block->setSamplingTime(samplingTime);
#ifdef QUA_ACCESS_CONTROL
		// permissions are optional (can be empty)
		auto permsBrowseName = listCols.at(6).trimmed();
		if (!permsBrowseName.isEmpty())
		{
			auto permsList = this->getPermissionsList();
			if (!permsList) { continue; }
			auto perms = permsList->permission(permsBrowseName);
			if (perms)
			{
				block->setPermissionsObject(perms);
			}
			else
			{
				errorLogs << QUaLog(
					tr("Failed to find '%1' permissions object after adding row [%2].").arg(permsBrowseName).arg(strRow),
					QUaLogLevel::Error,
					QUaLogCategory::Serialization
				);
			}
		}
#endif // QUA_ACCESS_CONTROL
	}
	return errorLogs;
}

QQueue<QUaLog> QUaModbusClientList::setCsvValues(QString strCsvValues)
{
	QQueue<QUaLog> errorLogs;
	auto listRows = strCsvValues.split("\n");
	for (auto strRow : listRows)
	{
		bool bOK = false;
		auto listCols = strRow.split(",");
		// check length
		if (listCols.count() <= 1)
		{
			continue;
		}
#ifndef QUA_ACCESS_CONTROL
		if (listCols.count() < 5)
#else
		if (listCols.count() < 6)
#endif // !QUA_ACCESS_CONTROL
		{
			errorLogs << QUaLog(
				tr("Invalid column count in row [%1]. Ignoring.").arg(strRow),
				QUaLogLevel::Warning,
				QUaLogCategory::Serialization
			);
			continue;
		}
		// get name
		auto strBrowseName = listCols.at(0).trimmed();
		// ignore row if first col is "Name"
		if (strBrowseName.compare("Name", Qt::CaseSensitive) == 0)
		{
			continue;
		}
		// get client
		auto strClientName = listCols.at(1).trimmed();
		auto client = this->browseChild<QUaModbusClient>(strClientName);
		if (!client)
		{
			errorLogs << QUaLog(
				tr("There is no client '%1' defined in row [%2].").arg(strClientName).arg(strRow),
				QUaLogLevel::Error,
				QUaLogCategory::Serialization
			);
			continue;
		}
		// get block
		auto strBlockName = listCols.at(2).trimmed();
		auto block = client->dataBlocks()->browseChild<QUaModbusDataBlock>(strBlockName);
		if (!block)
		{
			errorLogs << QUaLog(
				tr("There is no block '%1' defined in row [%2].").arg(strBlockName).arg(strRow),
				QUaLogLevel::Error,
				QUaLogCategory::Serialization
			);
			continue;
		}
		// get type
		auto type = (QModbusValueType)QMetaEnum::fromType<QModbusValueType>().keysToValue(listCols.at(3).trimmed().toUtf8(), &bOK);
		if (!bOK)
		{
			errorLogs << QUaLog(
				tr("Invalid Type '%1' in row [%2]. Default value set.").arg(listCols.at(3).trimmed()).arg(strRow),
				QUaLogLevel::Warning,
				QUaLogCategory::Serialization
			);
		}
		// get address offset
		auto addressOffset = listCols.at(4).trimmed().toInt(&bOK);
		if (!bOK)
		{
			errorLogs << QUaLog(
				tr("Invalid AddressOffset '%1' in row [%2]. Default value set.").arg(listCols.at(4).trimmed()).arg(strRow),
				QUaLogLevel::Warning,
				QUaLogCategory::Serialization
			);
		}
		auto values = block->values();
		auto value  = values->browseChild<QUaModbusValue>(strBrowseName);
		if (value)
		{
			errorLogs << QUaLog(
				tr("There already exists a value '%1.%2.%3' defined in row [%4]. Overwriting properties.").arg(client->browseName().name()).arg(block->browseName().name()).arg(strBrowseName).arg(strRow),
				QUaLogLevel::Warning,
				QUaLogCategory::Serialization
			);
		}
		else
		{
			// actually add block
			auto strNewError = values->addValue(strBrowseName);
			if (strNewError.contains("Error", Qt::CaseInsensitive))
			{
				errorLogs << QUaLog(
					strNewError,
					QUaLogLevel::Error,
					QUaLogCategory::Serialization
				);
				continue;
			}
			value = values->browseChild<QUaModbusValue>(strBrowseName);
			if (!value)
			{
				errorLogs << QUaLog(
					tr("Failed to find '%2' value in client %2, block '%3' list after adding row [%4].").arg(client->browseName().name()).arg(block->browseName().name()).arg(strBrowseName).arg(strRow),
					QUaLogLevel::Error,
					QUaLogCategory::Serialization
				);
				continue;
			}
		}
		// set properties
		value->setType(type);
		value->setAddressOffset(addressOffset);
#ifdef QUA_ACCESS_CONTROL
		// permissions are optional (can be empty)
		auto permsBrowseName = listCols.at(5).trimmed();
		if (!permsBrowseName.isEmpty())
		{
			auto permsList = this->getPermissionsList();
			if (!permsList) { continue; }
			auto perms = permsList->permission(permsBrowseName);
			if (perms)
			{
				value->setPermissionsObject(perms);
			}
			else
			{
				errorLogs << QUaLog(
					tr("Failed to find '%1' permissions object after adding row [%2].").arg(permsBrowseName).arg(strRow),
					QUaLogLevel::Error,
					QUaLogCategory::Serialization
				);
			}
		}
#endif // QUA_ACCESS_CONTROL
	}
	return errorLogs;
}

QList<QUaModbusClient*> QUaModbusClientList::clients()
{
	return this->browseChildren<QUaModbusClient>();
}

void QUaModbusClientList::clearInmediatly()
{
	for (auto client : this->clients())
	{
		delete client;
	}
}

#ifdef QUA_ACCESS_CONTROL
QUaPermissionsList * QUaModbusClientList::getPermissionsList()
{
	auto permsClientList = this->permissionsObject();
	Q_ASSERT_X(permsClientList, "QUaModbusClientList::getPermissionsList",
		"QUaModbusClientList must have a permissions object assigned in order to assign permissions to children.");
	if (!permsClientList)
	{
		return nullptr;
	}
	auto permsList = permsClientList->list();
	Q_CHECK_PTR(permsList);
	return permsList;
}
#endif // QUA_ACCESS_CONTROL

QDomElement QUaModbusClientList::toDomElement(QDomDocument & domDoc) const
{
	// add client list element
	QDomElement elemListClients = domDoc.createElement(QUaModbusClientList::staticMetaObject.className());
#ifdef QUA_ACCESS_CONTROL
	// set parmissions if any
	if (this->hasPermissionsObject())
	{
		elemListClients.setAttribute("Permissions", this->permissionsObject()->nodeId());
	}
#endif // QUA_ACCESS_CONTROL
	// loop children and add them as children
	auto clients = this->browseChildren<QUaModbusClient>();
	for (auto client : clients)
	{
		QDomElement elemClient = client->toDomElement(domDoc);
		elemListClients.appendChild(elemClient);
	}
	// return list element
	return elemListClients;
}

void QUaModbusClientList::fromDomElement(QDomElement & domElem, QQueue<QUaLog>& errorLogs)
{
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
	// add TCP clients
	QDomNodeList listTcpClients = domElem.elementsByTagName(QUaModbusTcpClient::staticMetaObject.className());
	for (int i = 0; i < listTcpClients.count(); i++)
	{
		QDomElement elemClient = listTcpClients.at(i).toElement();
		Q_ASSERT(!elemClient.isNull());
		if (!elemClient.hasAttribute("BrowseName"))
		{
			errorLogs << QUaLog(
				tr("Cannot add TCP client without BrowseName attribute. Skipping."),
				QUaLogLevel::Error,
				QUaLogCategory::Serialization
			);
			continue;
		}
		QString strBrowseName = elemClient.attribute("BrowseName");
		if (strBrowseName.isEmpty())
		{
			errorLogs << QUaLog(
				tr("Cannot add TCP client with empty BrowseName attribute. Skipping."),
				QUaLogLevel::Error,
				QUaLogCategory::Serialization
			);
			continue;
		}
		// check if exists
		auto client = this->browseChild<QUaModbusClient>(strBrowseName);
		if (client)
		{
			errorLogs << QUaLog(
				tr("Modbus client with %1 BrowseName already exists. Merging client configuration.").arg(strBrowseName),
				QUaLogLevel::Warning,
				QUaLogCategory::Serialization
			);
			// merge client config
			client->fromDomElement(elemClient, errorLogs);
			continue;
		}
		this->addClient<QUaModbusTcpClient>(strBrowseName);
		client = this->browseChild<QUaModbusTcpClient>(strBrowseName);
		if (!client)
		{
			errorLogs << QUaLog(
				tr("Failed to create TCP client with %1 BrowseName. Skipping.").arg(strBrowseName),
				QUaLogLevel::Error,
				QUaLogCategory::Serialization
			);
			continue;
		}
		// set client config
		client->fromDomElement(elemClient, errorLogs);
		// connect if keepConnecting is set
		if (client->keepConnecting()->value().toBool())
		{
			client->connectDevice();
		}
	}
	// add Serial clients
	QDomNodeList listSerialClients = domElem.elementsByTagName(QUaModbusRtuSerialClient::staticMetaObject.className());
	for (int i = 0; i < listSerialClients.count(); i++)
	{
		QDomElement elemClient = listSerialClients.at(i).toElement();
		Q_ASSERT(!elemClient.isNull());
		if (!elemClient.hasAttribute("BrowseName"))
		{
			errorLogs << QUaLog(
				tr("Cannot add Serial client without BrowseName attribute. Skipping."),
				QUaLogLevel::Error,
				QUaLogCategory::Serialization
			);
			continue;
		}
		QString strBrowseName = elemClient.attribute("BrowseName");
		if (strBrowseName.isEmpty())
		{
			errorLogs << QUaLog(
				tr("Cannot add Serial client with empty BrowseName. Skipping."),
				QUaLogLevel::Error,
				QUaLogCategory::Serialization
			);
			continue;
		}
		// check if exists
		auto client = this->browseChild<QUaModbusClient>(strBrowseName);
		if (client)
		{
			errorLogs << QUaLog(
				tr("Modbus client with %1 BrowseName already exists. Skipping.").arg(strBrowseName),
				QUaLogLevel::Error,
				QUaLogCategory::Serialization
			);
			continue;
		}
		this->addClient<QUaModbusRtuSerialClient>(strBrowseName);
		client = this->browseChild<QUaModbusRtuSerialClient>(strBrowseName);
		if (!client)
		{
			errorLogs << QUaLog(
				tr("Failed to create Serial client with %1 BrowseName. Skipping.").arg(strBrowseName),
				QUaLogLevel::Error,
				QUaLogCategory::Serialization
			);
			continue;
		}
		// set client config
		client->fromDomElement(elemClient, errorLogs);
		// connect if keepConnecting is set
		if (client->keepConnecting()->value().toBool())
		{
			client->connectDevice();
		}
	}
}