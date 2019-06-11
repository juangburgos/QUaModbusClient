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
	this->toDomElement(doc);
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

QList<QUaModbusClient*> QUaModbusClientList::clients()
{
	return this->browseChildren<QUaModbusClient>();
}

QDomElement QUaModbusClientList::toDomElement(QDomDocument & domDoc) const
{
	// add client list element
	QDomElement elemListClients = domDoc.createElement(QUaModbusClientList::staticMetaObject.className());
	domDoc.appendChild(elemListClients);
	// set attributes
	elemListClients.setAttribute("BrowseName", this->browseName());
	// loop children and add them as children
	auto clients = this->browseChildren<QUaModbusClient>();
	for (int i = 0; i < clients.count(); i++)
	{
		auto client = clients.at(i);
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