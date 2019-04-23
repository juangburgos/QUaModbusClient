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

QDomElement QUaModbusValueList::toDomElement(QDomDocument & domDoc) const
{
	// add value list element
	QDomElement elemListValues = domDoc.createElement(QUaModbusValueList::metaObject()->className());
	domDoc.appendChild(elemListValues);
	// loop children and add them as children
	auto values = this->browseChildren<QUaModbusValue>();
	for (int i = 0; i < values.count(); i++)
	{
		auto value = values.at(i);
		QDomElement elemValue = value->toDomElement(domDoc);
		elemListValues.appendChild(elemValue);
	}
	// return value element
	return elemListValues;
}

void QUaModbusValueList::fromDomElement(QDomElement & domElem, QString & strError)
{
	// TODO
}
