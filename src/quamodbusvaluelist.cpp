#include "quamodbusvaluelist.h"
#include "quamodbusdatablock.h"
#include "quamodbusvalue.h"

// NOTE : had to add this header because the actual implementation of QUaBaseObject::addChild is in here
//        and was getting "lnk2019 unresolved external symbol template function" without it
#include <QUaServer>

QUaModbusValueList::QUaModbusValueList(QUaServer *server)
	: QUaFolderObject(server)
{
	// NOTE : QObject parent might not be yet available in constructor
}

QString QUaModbusValueList::addValue(QString strValueId)
{
	strValueId = strValueId.trimmed();
	// check empty
	if (strValueId.isEmpty())
	{
		return "Error : Value Id argument cannot be empty.";
	}
	// check if id already exists
	if (this->hasChild(strValueId))
	{
		return "Error : Value Id already exists.";
	}
	// create instance
	auto block = this->addChild<QUaModbusValue>();
	block->setDisplayName(strValueId);
	block->setBrowseName(strValueId);
	// return
	return "Success";
}

QUaModbusDataBlock * QUaModbusValueList::block()
{
	return dynamic_cast<QUaModbusDataBlock*>(this->parent());
}

QList<QUaModbusValue*> QUaModbusValueList::values()
{
	return this->browseChildren<QUaModbusValue>();
}
