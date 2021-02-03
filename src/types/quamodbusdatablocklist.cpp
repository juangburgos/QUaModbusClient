#include "quamodbusdatablocklist.h"
#include "quamodbusclient.h"
#include "quamodbusdatablock.h"

// NOTE : had to add this header because the actual implementation of QUaBaseObject::addChild is in here
//        and was getting "lnk2019 unresolved external symbol template function" without it
#include <QUaServer>

#include <QRegularExpression>
#include <QRegularExpressionMatch>

#ifdef QUA_ACCESS_CONTROL
#include <QUaPermissions>
#endif // QUA_ACCESS_CONTROL

QUaModbusDataBlockList::QUaModbusDataBlockList(QUaServer *server)
#ifndef QUA_ACCESS_CONTROL
	: QUaFolderObject(server)
#else
	: QUaFolderObjectProtected(server)
#endif // !QUA_ACCESS_CONTROL
{
	// NOTE : QObject parent might not be yet available in constructor
}

QString QUaModbusDataBlockList::addDataBlock(const QUaQualifiedName& blockId)
{
	auto strBlockId = blockId.name();
	// check empty
	if (strBlockId.isEmpty())
	{
		return tr("%1 : Block Id argument cannot be empty.").arg("Error");
	}
	// check valid length
	if (strBlockId.count() > 40)
	{
		return  tr("%1 : Block Id cannot contain more than 40 characters.").arg("Error");
	}
	// check not called Name
	if (strBlockId.compare("Name", Qt::CaseSensitive) == 0)
	{
		return tr("%1 : Block Id cannot be 'Name'.").arg("Error");
	}
	// check valid characters
	QRegularExpression rx("^[a-zA-Z0-9_]*$");
	QRegularExpressionMatch match = rx.match(strBlockId, 0, QRegularExpression::PartialPreferCompleteMatch);
	if (!match.hasMatch())
	{
		return  tr("%1 : Block Id can only contain numbers, letters and underscores /^[a-zA-Z0-9_]*$/.").arg("Error");
	}
	// check if id already exists
	if (this->hasChild(blockId))
	{
		return  tr("%1 : Block Id already exists.").arg("Error");
	}
	// create instance
	QUaNodeId nodeId = { 0, QString("modbus.%1.%2").arg(this->client()->browseName().name()).arg(strBlockId) };
	auto block = this->addChild<QUaModbusDataBlock>(blockId, nodeId);
	if (!block)
	{
		return  tr("%1 : NodeId %2 already exists.").arg("Error").arg(nodeId);
	}
	// start block loop
	block->startLoop();
	// return
	return "Success";
}

void QUaModbusDataBlockList::clear()
{
	emit this->aboutToClear();
	for (auto &block : this->blocks())
	{
		block->remove();
	}
}

QUaModbusClient * QUaModbusDataBlockList::client()
{
	return qobject_cast<QUaModbusClient*>(this->parent());
}

QList<QUaModbusDataBlock*> QUaModbusDataBlockList::blocks()
{
	return this->browseChildren<QUaModbusDataBlock>();
}

QDomElement QUaModbusDataBlockList::toDomElement(QDomDocument & domDoc) const
{
	// add block list element
	QDomElement elemListBlocks = domDoc.createElement(QUaModbusDataBlockList::staticMetaObject.className());
	domDoc.appendChild(elemListBlocks);
#ifdef QUA_ACCESS_CONTROL
	// set parmissions if any
	if (this->hasPermissionsObject())
	{
		elemListBlocks.setAttribute("Permissions", this->permissionsObject()->nodeId());
	}
#endif // QUA_ACCESS_CONTROL
	// loop children and add them as children
	auto blocks = this->browseChildren<QUaModbusDataBlock>();
	for (int i = 0; i < blocks.count(); i++)
	{
		auto block = blocks.at(i);
		QDomElement elemBlock = block->toDomElement(domDoc);
		elemListBlocks.appendChild(elemBlock);
	}
	// return block element
	return elemListBlocks;
}

void QUaModbusDataBlockList::fromDomElement(QDomElement & domElem, QQueue<QUaLog>& errorLogs)
{
#ifdef QUA_ACCESS_CONTROL
	// load permissions if any
	if (domElem.hasAttribute("Permissions") && !domElem.attribute("Permissions").isEmpty())
	{
		QString strError = this->setPermissions(domElem.attribute("Permissions"));
		if (strError.contains("Error"))
		{
			errorLogs << QUaLog(
				strError,
				QUaLogLevel::Error,
				QUaLogCategory::Serialization
			);
		}
	}
#endif // QUA_ACCESS_CONTROL
	// add blocks
	QDomNodeList listBlocks = domElem.elementsByTagName(QUaModbusDataBlock::staticMetaObject.className());
	for (int i = 0; i < listBlocks.count(); i++)
	{
		QDomElement elemBlock = listBlocks.at(i).toElement();
		Q_ASSERT(!elemBlock.isNull());
		if (!elemBlock.hasAttribute("BrowseName"))
		{
			errorLogs << QUaLog(
				tr("Cannot add Block without BrowseName attribute. Skipping."),
				QUaLogLevel::Error,
				QUaLogCategory::Serialization
			);
			continue;
		}
		QString strBrowseName = elemBlock.attribute("BrowseName");
		if (strBrowseName.isEmpty())
		{
			errorLogs << QUaLog(
				tr("Cannot add Block with empty BrowseName attribute. Skipping."),
				QUaLogLevel::Error,
				QUaLogCategory::Serialization
			);
			continue;
		}
		// check if exists
		auto block = this->browseChild<QUaModbusDataBlock>(strBrowseName);
		if (block)
		{
			errorLogs << QUaLog(
				tr("Block with %1 BrowseName already exists. Overwriting Block configuration.").arg(strBrowseName),
				QUaLogLevel::Warning,
				QUaLogCategory::Serialization
			);
			// overwrite block config
			// NOTE : loop already should have started
			block->fromDomElement(elemBlock, errorLogs);
			continue;
		}
		this->addDataBlock(strBrowseName);
		block = this->browseChild<QUaModbusDataBlock>(strBrowseName);
		if (!block)
		{
			errorLogs << QUaLog(
				tr("Failed to create Block with %1 BrowseName. Skipping.").arg(strBrowseName),
				QUaLogLevel::Error,
				QUaLogCategory::Serialization
			);
			continue;
		}
		// set block config
		block->fromDomElement(elemBlock, errorLogs);
		// start block loop if setting samplingTime didn't
		if (!block->loopRunning())
		{
			block->startLoop();
		}
	}
}
