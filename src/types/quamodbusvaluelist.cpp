#include "quamodbusvaluelist.h"
#include "quamodbusdatablock.h"
#include "quamodbusvalue.h"

// NOTE : had to add this header because the actual implementation of QUaBaseObject::addChild is in here
//        and was getting "lnk2019 unresolved external symbol template function" without it
#include <QUaServer>

#include <QRegularExpression>
#include <QRegularExpressionMatch>

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
	// check valid length
	if (strValueId.count() > 10)
	{
		return "Error : Value Id cannot contain more than 6 characters.";
	}
	// check valid characters
	QRegularExpression rx("^[a-zA-Z0-9_]*$");
	QRegularExpressionMatch match = rx.match(strValueId, 0, QRegularExpression::PartialPreferCompleteMatch);
	if (!match.hasMatch())
	{
		return "Error : Value Id can only contain numbers, letters and underscores /^[a-zA-Z0-9_]*$/.";
	}
	// check if id already exists
	if (this->hasChild(strValueId))
	{
		return "Error : Value Id already exists.";
	}
	// create instance
	// TODO : set custom nodeId when https://github.com/open62541/open62541/issues/2667 fixed
	//QString strNodeId = QString("ns=1;s=%1").arg(this->nodeBrowsePath().join(".") + "." + strClientId);
	auto block = this->addChild<QUaModbusValue>(/*strNodeId*/);
	block->setDisplayName(strValueId);
	block->setBrowseName(strValueId);
	// return
	return "Success";
}

void QUaModbusValueList::clear()
{
	for (int i = 0; i < this->values().count(); i++)
	{
		this->values().at(i)->remove();
	}
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
	QDomElement elemListValues = domDoc.createElement(QUaModbusValueList::staticMetaObject.className());
	domDoc.appendChild(elemListValues);
	// set attributes
	elemListValues.setAttribute("BrowseName", this->browseName());
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
	// add values
	QDomNodeList listValues = domElem.elementsByTagName(QUaModbusValue::staticMetaObject.className());
	for (int i = 0; i < listValues.count(); i++)
	{
		QDomElement elemValue = listValues.at(i).toElement();
		Q_ASSERT(!elemValue.isNull());
		if (!elemValue.hasAttribute("BrowseName"))
		{
			strError += "Error : Cannot add Value without BrowseName attribute. Skipping.\n";
			continue;
		}
		QString strBrowseName = elemValue.attribute("BrowseName");
		if (strBrowseName.isEmpty())
		{
			strError += "Error : Cannot add Value with empty BrowseName attribute. Skipping.\n";
			continue;
		}
		// check if exists
		auto value = this->browseChild<QUaModbusValue>(strBrowseName);
		if (value)
		{
			strError += QString("Warning : Value with %1 BrowseName already exists. Overwriting Value configuration.\n").arg(strBrowseName);
			// overwrite value config
			value->fromDomElement(elemValue, strError);
			continue;
		}
		this->addValue(strBrowseName);
		value = this->browseChild<QUaModbusValue>(strBrowseName);
		if (!value)
		{
			strError += QString("Error : Failed to create Value with %1 BrowseName. Skipping.\n").arg(strBrowseName);
			continue;
		}
		// set value config
		value->fromDomElement(elemValue, strError);
	}
}
