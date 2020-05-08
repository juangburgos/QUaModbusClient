#include "quamodbusvaluelist.h"
#include "quamodbusdatablock.h"
#include "quamodbusvalue.h"
#include "quamodbusclient.h"

// NOTE : had to add this header because the actual implementation of QUaBaseObject::addChild is in here
//        and was getting "lnk2019 unresolved external symbol template function" without it
#include <QUaServer>

#include <QRegularExpression>
#include <QRegularExpressionMatch>

#ifdef QUA_ACCESS_CONTROL
#include <QUaPermissions>
#endif // QUA_ACCESS_CONTROL

QUaModbusValueList::QUaModbusValueList(QUaServer *server)
#ifndef QUA_ACCESS_CONTROL
	: QUaFolderObject(server)
#else
	: QUaFolderObjectProtected(server)
#endif // !QUA_ACCESS_CONTROL
{
	// NOTE : QObject parent might not be yet available in constructor
}

QString QUaModbusValueList::addValue(const QUaQualifiedName& valueId)
{
	auto strValueId = valueId.name();
	// check empty
	if (strValueId.isEmpty())
	{
		return tr("%1 : Value Id argument cannot be empty.").arg("Error");
	}
	// check valid length
	if (strValueId.count() > 130)
	{
		return tr("%1 : Value Id cannot contain more than 120 characters.").arg("Error");
	}
	// check not called Name
	if (strValueId.compare("Name", Qt::CaseSensitive) == 0)
	{
		return tr("%1 : Value Id cannot be 'Name'.").arg("Error");
	}
	// check valid characters
	QRegularExpression rx("^[a-zA-Z0-9_]*$");
	QRegularExpressionMatch match = rx.match(strValueId, 0, QRegularExpression::PartialPreferCompleteMatch);
	if (!match.hasMatch())
	{
		return tr("%1 : Value Id can only contain numbers, letters and underscores /^[a-zA-Z0-9_]*$/.").arg("Error");
	}
	// check if id already exists
	if (this->hasChild(valueId))
	{
		return tr("%1 : Value Id already exists.").arg("Error");
	}
	// create instance
	QUaNodeId strNodeId = { 
		0, 
		QString("modbus.%1.%2.%3")
			.arg(this->block()->client()->browseName().name())
			.arg(this->block()->browseName().name())
			.arg(strValueId) 
	};
	auto value = this->addChild<QUaModbusValue>(valueId, strNodeId);
	if (!value)
	{
		return  tr("%1 : NodeId %2 already exists.").arg("Error").arg(strNodeId);
	}
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
#ifdef QUA_ACCESS_CONTROL
	// set parmissions if any
	if (this->hasPermissionsObject())
	{
		elemListValues.setAttribute("Permissions", this->permissionsObject()->nodeId());
	}
#endif // QUA_ACCESS_CONTROL
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
#ifdef QUA_ACCESS_CONTROL
	// load permissions if any
	if (domElem.hasAttribute("Permissions") && !domElem.attribute("Permissions").isEmpty())
	{
		strError += this->setPermissions(domElem.attribute("Permissions"));
	}
#endif // QUA_ACCESS_CONTROL
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
