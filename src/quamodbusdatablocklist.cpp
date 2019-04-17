#include "quamodbusdatablocklist.h"
#include "quamodbusclient.h"
#include "quamodbusdatablock.h"

// NOTE : had to add this header because the actual implementation of QUaBaseObject::addChild is in here
//        and was getting "lnk2019 unresolved external symbol template function" without it
#include <QUaServer>

QUaModbusDataBlockList::QUaModbusDataBlockList(QUaServer *server)
	: QUaFolderObject(server)
{
	// NOTE : QObject parent might not be yet available in constructor
}

QString QUaModbusDataBlockList::addDataBlock(QString strBlockId)
{
	strBlockId = strBlockId.trimmed();
	// check empty
	if (strBlockId.isEmpty())
	{
		return "Error : Block Id argument cannot be empty.";
	}
	// check if id already exists
	if (this->hasChild(strBlockId))
	{
		return "Error : Block Id already exists.";
	}
	// create instance
	auto block = this->addChild<QUaModbusDataBlock>();
	block->setDisplayName(strBlockId);
	block->setBrowseName(strBlockId);
	// start block loop
	block->startLoop();
	// return
	return "Success";
}

QUaModbusClient * QUaModbusDataBlockList::client()
{
	return dynamic_cast<QUaModbusClient*>(this->parent());
}
