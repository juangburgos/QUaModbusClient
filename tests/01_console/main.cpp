#include <QCoreApplication>
#include <QDebug>

#include <QUaServer>

#include <QUaModbusClientList>

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	QUaServer server;

	QUaFolderObject * objsFolder = server.objectsFolder();

	// add list entry point to object's folder
	objsFolder->addChild<QUaModbusClientList>("ModbusClients");

	server.start();

	return a.exec(); 
}
