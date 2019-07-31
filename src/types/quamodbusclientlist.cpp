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

QString QUaModbusClientList::addTcpClient(QString strClientId)
{
	return this->addClient<QUaModbusTcpClient>(strClientId);
}

QString QUaModbusClientList::addRtuSerialClient(QString strClientId)
{
	return this->addClient<QUaModbusRtuSerialClient>(strClientId);
}

void QUaModbusClientList::clear()
{
	for (int i = 0; i < this->clients().count(); i++)
	{
		this->clients().at(i)->remove();
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
	QString strError;
	// set to dom doc
	QDomDocument doc;
	int line, col;
	doc.setContent(strXmlConfig, &strError, &line, &col);
	if (!strError.isEmpty())
	{
		strError = tr("%1 : Invalid XML in Line %2 Column %3 Error %4").arg("Error").arg(line).arg(col).arg(strError);
		return strError;
	}
	// get list of clients
	QDomElement elemClientList = doc.firstChildElement(QUaModbusClientList::staticMetaObject.className());
	if (elemClientList.isNull())
	{
		strError = tr("%1 : No Modbus client list found in XML config.").arg("Error");
		return strError;
	}
	// load config from xml
	this->fromDomElement(elemClientList, strError);
	if (strError.isEmpty())
	{
		strError = "Success.";
	}
	return strError;
}

QString QUaModbusClientList::csvClients()
{
	QString strCsv;
	strCsv += QString("%1, %2, %3, %4, %5, %6\n")
		.arg(tr("Name"))
		.arg(tr("Type"))
		.arg(tr("ServerAddress"))
		.arg(tr("KeepConnecting"))
		.arg(tr("NetworkAddress"))
		.arg(tr("NetworkPort"));
	auto clientsTcp = this->browseChildren<QUaModbusTcpClient>();
	for (auto clientTcp : clientsTcp)
	{
		auto strType = QString(QMetaEnum::fromType<QModbusClientType>().valueToKey(clientTcp->getType()));
		strCsv += QString("%1, %2, %3, %4, %5, %6\n")
			.arg(clientTcp->browseName())
			.arg(strType)
			.arg(clientTcp->getServerAddress())
			.arg(clientTcp->getKeepConnecting())
			.arg(clientTcp->getNetworkAddress())
			.arg(clientTcp->getNetworkPort());
	}
	strCsv += QString("%1, %2, %3, %4, %5, %6, %7, %8, %9\n")
		.arg(tr("Name"))
		.arg(tr("Type"))
		.arg(tr("ServerAddress"))
		.arg(tr("KeepConnecting"))
		.arg(tr("ComPort"))
		.arg(tr("Parity"))
		.arg(tr("BaudRate"))
		.arg(tr("DataBits"))
		.arg(tr("StopBits"));
	auto clientsSerial = this->browseChildren<QUaModbusRtuSerialClient>();
	for (auto clientSerial : clientsSerial)
	{
		auto strType = QString(QMetaEnum::fromType<QModbusClientType>().valueToKey(clientSerial->getType()));
		auto strParity = QString(QMetaEnum::fromType<QParity>().valueToKey(clientSerial->getParity()));
		auto strBaudRate = QString(QMetaEnum::fromType<QBaudRate>().valueToKey(clientSerial->getBaudRate()));
		auto strDataBits = QString(QMetaEnum::fromType<QDataBits>().valueToKey(clientSerial->getDataBits()));
		auto strStopBits = QString(QMetaEnum::fromType<QStopBits>().valueToKey(clientSerial->getStopBits()));
		strCsv += QString("%1, %2, %3, %4, %5, %6, %7, %8, %9\n")
			.arg(clientSerial->browseName())
			.arg(strType)
			.arg(clientSerial->getServerAddress())
			.arg(clientSerial->getKeepConnecting())
			.arg(clientSerial->getComPort())
			.arg(strParity)
			.arg(strBaudRate)
			.arg(strDataBits)
			.arg(strStopBits);
	}
	return strCsv;
}

QString QUaModbusClientList::csvBlocks()
{
	QString strCsv;
	strCsv += QString("%1, %2, %3, %4, %5, %6\n")
		.arg(tr("Name"))
		.arg(tr("Client"))
		.arg(tr("Type"))
		.arg(tr("Address"))
		.arg(tr("Size"))
		.arg(tr("SamplingTime"));
	auto clients = this->browseChildren<QUaModbusClient>();
	for (auto client : clients)
	{
		auto blocks = client->dataBlocks()->blocks();
		for (auto block : blocks)
		{
			auto strType = QString(QMetaEnum::fromType<QModbusDataBlockType>().valueToKey(block->getType()));
			strCsv += QString("%1, %2, %3, %4, %5, %6\n")
				.arg(block->browseName())
				.arg(client->browseName())
				.arg(strType)
				.arg(block->getAddress())
				.arg(block->getSize())
				.arg(block->getSamplingTime());
		}
	}
	return strCsv;
}

QString QUaModbusClientList::csvValues()
{
	QString strCsv;
	strCsv += QString("%1, %2, %3, %4, %5\n")
		.arg(tr("Name"))
		.arg(tr("Client"))
		.arg(tr("Block"))
		.arg(tr("Type"))
		.arg(tr("AddressOffset"));
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
				strCsv += QString("%1, %2, %3, %4, %5\n")
					.arg(value->browseName())
					.arg(client->browseName())
					.arg(block->browseName())
					.arg(strType)
					.arg(value->getAddressOffset());
			}
		}
	}
	return strCsv;
}

QString QUaModbusClientList::setCsvClients(QString strCsvClients)
{
	QString strError;
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
		if (listCols.count() < 6)
		{
			strError += tr("%1 : Invalid column count in row %2. Ignoring\n")
				.arg("Warning")
				.arg(strRow);
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
			strError += tr("%1 : Invalid client type '%2' in row %3.\n")
				.arg("Error")
				.arg(strType)
				.arg(strRow);
			continue;
		}
		// get modbus address
		auto serverAddress = listCols.at(2).trimmed().toUInt(&bOK);
		if (!bOK)
		{
			strError += tr("%1 : Invalid ServerAddress '%2' in row %3. Default value set.\n")
				.arg("Warning")
				.arg(listCols.at(2).trimmed())
				.arg(strRow);
		}
		// get keep connecting
		auto keepConnecting = (bool)listCols.at(3).trimmed().toUInt(&bOK);
		if (!bOK)
		{
			strError += tr("%1 : Invalid KeepConnecting '%2' in row %3. Default value set.\n")
				.arg("Warning")
				.arg(listCols.at(3).trimmed())
				.arg(strRow);
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
					strError += tr("%1 : Invalid NetworkAddress '%2' in row %3. Default value set.\n")
						.arg("Warning")
						.arg(listCols.at(4).trimmed())
						.arg(strRow);
				}
				// get network port
				auto networkPort = listCols.at(5).trimmed().toUInt(&bOK);
				if (!bOK)
				{
					strError += tr("%1 : Invalid NetworkPort '%2' in row %3. Default value set.\n")
						.arg("Warning")
						.arg(listCols.at(5).trimmed())
						.arg(strRow);
				}
				// check if tcp client exists
				auto clientTcp = this->browseChild<QUaModbusTcpClient>(strBrowseName);
				if (clientTcp)
				{
					strError += tr("%1 : Client '%2' already exists in row %3. Overwriting properties.\n")
						.arg("Warning")
						.arg(strBrowseName)
						.arg(strRow);
				}
				else
				{
					// check if serial client exists
					auto clientSerial = this->browseChild<QUaModbusRtuSerialClient>(strBrowseName);
					if (clientSerial)
					{
						strError += tr("%1 : There already exists a serial client '%2' defined in row %3. Delete it first.\n")
							.arg("Error")
							.arg(strBrowseName)
							.arg(strRow);
						continue;
					}
					// actually add client
					auto strNewError = this->addTcpClient(strBrowseName);
					if (strNewError.contains("Error", Qt::CaseInsensitive))
					{
						strError += strNewError;
						continue;
					}
					clientTcp = this->browseChild<QUaModbusTcpClient>(strBrowseName);
					if (!clientTcp)
					{
						strError += tr("%1 : Failed to find '%2' client in client list after adding row %3.\n")
							.arg("Error")
							.arg(strBrowseName)
							.arg(strRow);
						continue;
					}
				}
				// set props
				clientTcp->setServerAddress(serverAddress);
				clientTcp->setKeepConnecting(keepConnecting);
				clientTcp->setNetworkAddress(networkAddress);
				clientTcp->setNetworkPort(networkPort);
			}
			break;
		case QModbusClientType::Serial:
			{
				// check length
				if (listCols.count() < 9)
				{
					strError += tr("%1 : Invalid column count in row %2.\n")
						.arg("Error")
						.arg(strRow);
					continue;
				}
				// get com port
				auto comPort = listCols.at(4).trimmed();
				if (comPort.isEmpty())
				{
					strError += tr("%1 : Invalid ComPort '%2' in row %3. Default value set.\n")
						.arg("Warning")
						.arg(comPort)
						.arg(strRow);
				}
				// get parity
				auto strParity = listCols.at(5).trimmed();
				auto parity = (QParity)QMetaEnum::fromType<QParity>().keysToValue(strParity.toUtf8(), &bOK);
				if (!bOK)
				{
					strError += tr("%1 : Invalid Parity '%2' in row %3. Default value set.\n")
						.arg("Warning")
						.arg(strParity)
						.arg(strRow);
				}
				// get baud rate
				auto strBaudRate = listCols.at(6).trimmed();
				auto baudRate = (QBaudRate)QMetaEnum::fromType<QBaudRate>().keysToValue(strBaudRate.toUtf8(), &bOK);
				if (!bOK)
				{
					strError += tr("%1 : Invalid BaudRate '%2' in row %3. Default value set.\n")
						.arg("Warning")
						.arg(strBaudRate)
						.arg(strRow);
				}
				// get data bits
				auto strDataBits = listCols.at(7).trimmed();
				auto dataBits = (QDataBits)QMetaEnum::fromType<QDataBits>().keysToValue(strDataBits.toUtf8(), &bOK);
				if (!bOK)
				{
					strError += tr("%1 : Invalid DataBits '%2' in row %3. Default value set.\n")
						.arg("Warning")
						.arg(strDataBits)
						.arg(strRow);
				}
				// get stop bits
				auto strStopBits = listCols.at(8).trimmed();
				auto stopBits = (QStopBits)QMetaEnum::fromType<QStopBits>().keysToValue(strStopBits.toUtf8(), &bOK);
				if (!bOK)
				{
					strError += tr("%1 : Invalid StopBits '%2' in row %3. Default value set.\n")
						.arg("Warning")
						.arg(strStopBits)
						.arg(strRow);
				}
				// check if serial client exists
				auto clientSerial = this->browseChild<QUaModbusRtuSerialClient>(strBrowseName);
				if (clientSerial)
				{
					strError += tr("%1 : Client '%2' already exists in row %3. Overwriting properties.\n")
						.arg("Warning")
						.arg(strBrowseName)
						.arg(strRow);
				}
				else
				{
					// check if tcp client exists
					auto clientTcp = this->browseChild<QUaModbusTcpClient>(strBrowseName);
					if (clientTcp)
					{
						strError += tr("%1 : There already exists a tcp client '%2' defined in row %3. Delete it first.\n")
							.arg("Error")
							.arg(strBrowseName)
							.arg(strRow);
						continue;
					}
					// actually add client
					auto strNewError = this->addRtuSerialClient(strBrowseName);
					if (strNewError.contains("Error", Qt::CaseInsensitive))
					{
						strError += strNewError;
						continue;
					}
					clientSerial = this->browseChild<QUaModbusRtuSerialClient>(strBrowseName);
					if (!clientSerial)
					{
						strError += tr("%1 : Failed to find '%2' client in client list after adding row %3.\n")
							.arg("Error")
							.arg(strBrowseName)
							.arg(strRow);
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
			}
			break;
		default:
			Q_ASSERT(false);
			break;
		}
	}
	if (strError.isEmpty())
	{
		strError = "Success.";
	}
	return strError;
}

QString QUaModbusClientList::setCsvBlocks(QString strCsvBlocks)
{
	QString strError;
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
		if (listCols.count() < 6)
		{
			strError += tr("%1 : Invalid column count in row %2. Ignoring\n")
				.arg("Warning")
				.arg(strRow);
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
			strError += tr("%1 : There is no client '%2' defined in row %3.\n")
				.arg("Error")
				.arg(strClientName)
				.arg(strRow);
			continue;
		}
		// get type
		auto type = (QModbusDataBlockType)QMetaEnum::fromType<QModbusDataBlockType>().keysToValue(listCols.at(2).trimmed().toUtf8(), &bOK);
		if (!bOK)
		{
			strError += tr("%1 : Invalid Type '%2' in row %3. Default value set.\n")
				.arg("Warning")
				.arg(listCols.at(2).trimmed()).
				arg(strRow);
		}
		// get address
		auto address = listCols.at(3).trimmed().toInt(&bOK);
		if (!bOK)
		{
			strError += tr("%1 : Invalid Address '%2' in row %3. Default value set.\n")
				.arg("Warning")
				.arg(listCols.at(3).trimmed())
				.arg(strRow);
		}
		// get size
		auto size = listCols.at(4).trimmed().toUInt(&bOK);
		if (!bOK)
		{
			strError += tr("%1 : Invalid Size '%2' in row %3. Default value set.\n")
				.arg("Warning")
				.arg(listCols.at(4).trimmed())
				.arg(strRow);
		}
		// get sampling time
		auto samplingTime = listCols.at(5).trimmed().toUInt(&bOK);
		if (!bOK)
		{
			strError += tr("%1 : Invalid SamplingTime '%2' in row %3. Default value set.\n")
				.arg("Warning")
				.arg(listCols.at(5).trimmed())
				.arg(strRow);
		}
		// check if block exists
		auto blocks = client->dataBlocks();
		auto block  = blocks->browseChild<QUaModbusDataBlock>(strBrowseName);
		if (block)
		{
			strError += tr("%1 : There already exists a block '%2.%3' defined in row %4. Overwriting properties.\n")
				.arg("Warning")
				.arg(client->browseName())
				.arg(strBrowseName)
				.arg(strRow);
		}
		else
		{
			// actually add block
			auto strNewError = blocks->addDataBlock(strBrowseName);
			if (strNewError.contains("Error", Qt::CaseInsensitive))
			{
				strError += strNewError;
				continue;
			}
			block = blocks->browseChild<QUaModbusDataBlock>(strBrowseName);
			if (!block)
			{
				strError += tr("%1 : Failed to find '%2' block in client '%3' list after adding row %4.\n")
					.arg("Error")
					.arg(client->browseName())
					.arg(strBrowseName)
					.arg(strRow);
				continue;
			}
		}	
		// set properties
		block->setType(type);
		block->setAddress(address);
		block->setSize(size);
		block->setSamplingTime(samplingTime);
	}
	if (strError.isEmpty())
	{
		strError = "Success.";
	}
	return strError;
}

QString QUaModbusClientList::setCsvValues(QString strCsvValues)
{
	QString strError;
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
		if (listCols.count() < 5)
		{
			strError += tr("%1 : Invalid column count in row %2. Ignoring\n")
				.arg("Warning")
				.arg(strRow);
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
			strError += tr("%1 : There is no client '%2' defined in row %3.\n")
				.arg("Error")
				.arg(strClientName)
				.arg(strRow);
			continue;
		}
		// get block
		auto strBlockName = listCols.at(2).trimmed();
		auto block = client->dataBlocks()->browseChild<QUaModbusDataBlock>(strBlockName);
		if (!block)
		{
			strError += tr("%1 : There is no block '%2' defined in row %3.\n")
				.arg("Error")
				.arg(strBlockName)
				.arg(strRow);
			continue;
		}
		// get type
		auto type = (QModbusValueType)QMetaEnum::fromType<QModbusValueType>().keysToValue(listCols.at(3).trimmed().toUtf8(), &bOK);
		if (!bOK)
		{
			strError += tr("%1 : Invalid Type '%2' in row %3. Default value set.\n")
				.arg("Warning")
				.arg(listCols.at(3).trimmed())
				.arg(strRow);
		}
		// get address offset
		auto addressOffset = listCols.at(4).trimmed().toInt(&bOK);
		if (!bOK)
		{
			strError += tr("%1 : Invalid AddressOffset '%1' in row %2. Default value set.\n")
				.arg("Warning")
				.arg(listCols.at(4).trimmed())
				.arg(strRow);
		}
		auto values = block->values();
		auto value  = values->browseChild<QUaModbusValue>(strBrowseName);
		if (value)
		{
			strError += tr("%1 : There already exists a value '%2.%3.%4' defined in row %5. Overwriting properties.\n")
				.arg("Warning")
				.arg(client->browseName())
				.arg(block->browseName())
				.arg(strBrowseName)
				.arg(strRow);
		}
		else
		{
			// actually add block
			auto strNewError = values->addValue(strBrowseName);
			if (strNewError.contains("Error", Qt::CaseInsensitive))
			{
				strError += strNewError;
				continue;
			}
			value = values->browseChild<QUaModbusValue>(strBrowseName);
			if (!value)
			{
				strError += tr("%1 : Failed to find '%2' value in client %3, block '%4' list after adding row %5.\n")
					.arg("Error")
					.arg(client->browseName())
					.arg(block->browseName())
					.arg(strBrowseName)
					.arg(strRow);
				continue;
			}
		}
		// set properties
		value->setType(type);
		value->setAddressOffset(addressOffset);
	}
	if (strError.isEmpty())
	{
		strError = "Success.";
	}
	return strError;
}

QList<QUaModbusClient*> QUaModbusClientList::clients()
{
	return this->browseChildren<QUaModbusClient>();
}

QDomElement QUaModbusClientList::toDomElement(QDomDocument & domDoc) const
{
	// add client list element
	QDomElement elemListClients = domDoc.createElement(QUaModbusClientList::staticMetaObject.className());
	// set attributes
	elemListClients.setAttribute("BrowseName", this->browseName());
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

void QUaModbusClientList::fromDomElement(QDomElement & domElem, QString & strError)
{
	// add TCP clients
	QDomNodeList listTcpClients = domElem.elementsByTagName(QUaModbusTcpClient::staticMetaObject.className());
	for (int i = 0; i < listTcpClients.count(); i++)
	{
		QDomElement elemClient = listTcpClients.at(i).toElement();
		Q_ASSERT(!elemClient.isNull());
		if (!elemClient.hasAttribute("BrowseName"))
		{
			strError += tr("%1 : Cannot add TCP client without BrowseName attribute. Skipping.\n").arg("Error");
			continue;
		}
		QString strBrowseName = elemClient.attribute("BrowseName");
		if (strBrowseName.isEmpty())
		{
			strError += tr("%1 : Cannot add TCP client with empty BrowseName attribute. Skipping.\n").arg("Error");
			continue;
		}
		// check if exists
		auto client = this->browseChild<QUaModbusClient>(strBrowseName);
		if (client)
		{
			strError += tr("%1 : Modbus client with %2 BrowseName already exists. Merging client configuration.\n").arg("Warning").arg(strBrowseName);
			// merge client config
			client->fromDomElement(elemClient, strError);
			continue;
		}
		this->addClient<QUaModbusTcpClient>(strBrowseName);
		client = this->browseChild<QUaModbusTcpClient>(strBrowseName);
		if (!client)
		{
			strError += tr("%1 : Failed to create TCP client with %2 BrowseName. Skipping.\n").arg("Error").arg(strBrowseName);
			continue;
		}
		// set client config
		client->fromDomElement(elemClient, strError);
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
			strError += tr("%1 : Cannot add Serial client without BrowseName attribute. Skipping.\n").arg("Error");
			continue;
		}
		QString strBrowseName = elemClient.attribute("BrowseName");
		if (strBrowseName.isEmpty())
		{
			strError += tr("%1 : Cannot add Serial client with empty BrowseName. Skipping.\n").arg("Error");
			continue;
		}
		// check if exists
		auto client = this->browseChild<QUaModbusClient>(strBrowseName);
		if (client)
		{
			strError += tr("%1 : Modbus client with %1 BrowseName already exists. Skipping.\n").arg("Error").arg(strBrowseName);
			continue;
		}
		this->addClient<QUaModbusRtuSerialClient>(strBrowseName);
		client = this->browseChild<QUaModbusRtuSerialClient>(strBrowseName);
		if (!client)
		{
			strError += tr("%1 : Failed to create Serial client with %1 BrowseName. Skipping.\n").arg("Error").arg(strBrowseName);
			continue;
		}
		// set client config
		client->fromDomElement(elemClient, strError);
		// connect if keepConnecting is set
		if (client->keepConnecting()->value().toBool())
		{
			client->connectDevice();
		}
	}
}