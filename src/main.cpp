#include <QCoreApplication>
#include <QDebug>

#include <QUaServer>

#include "quamodbusclient.h"

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	QUaServer server;

	QUaFolderObject * objsFolder = server.objectsFolder();

	// register modbus client type
	server.registerType<QUaModbusClient>();

	// method to add clients
	objsFolder->addMethod("addClient", [&objsFolder](QString strUniqueName) {
		strUniqueName = strUniqueName.trimmed();
		// check empty
		if (strUniqueName.isEmpty())
		{
			return "Error : Client Id argument cannot be empty.";
		}
		// check if id already exists
		if (objsFolder->hasChild(strUniqueName))
		{
			return "Error : Client Id already exists.";
		}
		// create instance
		auto client = objsFolder->addChild<QUaModbusClient>();
		client->setDisplayName(strUniqueName);
		client->setBrowseName(strUniqueName);
		return "Success";
	});

	// method to remove clients
	objsFolder->addMethod("removeClient", [&objsFolder](QString strUniqueName) {
		strUniqueName = strUniqueName.trimmed();
		// check empty
		if (strUniqueName.isEmpty())
		{
			return "Error : Client Id argument cannot be empty.";
		}
		// check if id already exists
		if (!objsFolder->hasChild(strUniqueName))
		{
			return "Error : Client Id does not exists.";
		}
		// create instance
		auto client = objsFolder->browseChild<QUaModbusClient>(strUniqueName);
		client->deleteLater();
		return "Success";
	});

	server.start();

	return a.exec(); 
}
