#include <QCoreApplication>
#include <QDebug>

#include <QUaServer>

#include "quamodbusclientlist.h"

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	QUaServer server;

	QUaFolderObject * objsFolder = server.objectsFolder();

	// add list entry point to object's folder
	auto modCliList = objsFolder->addChild<QUaModbusClientList>();
	modCliList->setDisplayName("ModbusClients");
	modCliList->setBrowseName ("ModbusClients");

	server.start();

	return a.exec(); 
}
