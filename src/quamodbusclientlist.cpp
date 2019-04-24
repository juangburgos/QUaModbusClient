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
	server->registerEnum<QModbusDevice::State    >();
	server->registerEnum<QModbusDevice::Error    >();
	server->registerEnum<QSerialPort::Parity     >();
	server->registerEnum<QSerialPort::BaudRate   >();
	server->registerEnum<QSerialPort::DataBits   >();
	server->registerEnum<QSerialPort::StopBits   >();
}

Q_INVOKABLE QString QUaModbusClientList::addTcpClient(QString strClientId)
{
	return this->addClient<QUaModbusTcpClient>(strClientId);
}

Q_INVOKABLE QString QUaModbusClientList::addRtuSerialClient(QString strClientId)
{
	return this->addClient<QUaModbusRtuSerialClient>(strClientId);
}

Q_INVOKABLE QString QUaModbusClientList::xmlConfig()
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
		strError = QString("Error : Invalid XML in Line %1 Column %2 Error %3").arg(line).arg(col).arg(strError);
		return strError;
	}
	// get list of clients
	QDomElement elemClientList = doc.firstChildElement(QUaModbusClientList::staticMetaObject.className());
	if (elemClientList.isNull())
	{
		strError = "Error : No Modbus client list found in XML config.";
		return strError;
	}
	// load config from xml
	this->fromDomElement(elemClientList, strError);
	if (strError.isEmpty())
	{
		strError = "Success";
	}
	return strError;
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
			strError += "Error : Cannot add TCP client without BrowseName attribute. Skipping.\n";
			continue;
		}
		QString strBrowseName = elemClient.attribute("BrowseName");
		if (strBrowseName.isEmpty())
		{
			strError += "Error : Cannot add TCP client with empty BrowseName attribute. Skipping.\n";
			continue;
		}
		// check if exists
		auto client = this->browseChild<QUaModbusClient>(strBrowseName);
		if (client)
		{
			strError += QString("Warning : Modbus client with %1 BrowseName already exists. Merging client configuration.\n").arg(strBrowseName);
			// merge client config
			client->fromDomElement(elemClient, strError);
			continue;
		}
		this->addClient<QUaModbusTcpClient>(strBrowseName);
		client = this->browseChild<QUaModbusTcpClient>(strBrowseName);
		if (!client)
		{
			strError += QString("Error : Failed to create TCP client with %1 BrowseName. Skipping.\n").arg(strBrowseName);
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
			strError += "Error : Cannot add Serial client without BrowseName attribute. Skipping.\n";
			continue;
		}
		QString strBrowseName = elemClient.attribute("BrowseName");
		if (strBrowseName.isEmpty())
		{
			strError += "Error : Cannot add Serial client with empty BrowseName. Skipping.\n";
			continue;
		}
		// check if exists
		auto client = this->browseChild<QUaModbusClient>(strBrowseName);
		if (client)
		{
			strError += QString("Error : Modbus client with %1 BrowseName already exists. Skipping.\n").arg(strBrowseName);
			continue;
		}
		this->addClient<QUaModbusRtuSerialClient>(strBrowseName);
		client = this->browseChild<QUaModbusRtuSerialClient>(strBrowseName);
		if (!client)
		{
			strError += QString("Error : Failed to create Serial client with %1 BrowseName. Skipping.\n").arg(strBrowseName);
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