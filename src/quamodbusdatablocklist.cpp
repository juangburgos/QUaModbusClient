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

QDomElement QUaModbusDataBlockList::toDomElement(QDomDocument & domDoc) const
{
	// add block list element
	QDomElement elemListBlocks = domDoc.createElement(QUaModbusDataBlockList::staticMetaObject.className());
	domDoc.appendChild(elemListBlocks);
	// set attributes
	elemListBlocks.setAttribute("BrowseName", this->browseName());
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

void QUaModbusDataBlockList::fromDomElement(QDomElement & domElem, QString & strError)
{
	// add blocks
	QDomNodeList listBlocks = domElem.elementsByTagName(QUaModbusDataBlock::staticMetaObject.className());
	for (int i = 0; i < listBlocks.count(); i++)
	{
		QDomElement elemBlock = listBlocks.at(i).toElement();
		Q_ASSERT(!elemBlock.isNull());
		if (!elemBlock.hasAttribute("BrowseName"))
		{
			strError += "Error : Cannot add Block without BrowseName attribute. Skipping.\n";
			continue;
		}
		QString strBrowseName = elemBlock.attribute("BrowseName");
		if (strBrowseName.isEmpty())
		{
			strError += "Error : Cannot add Block with empty BrowseName attribute. Skipping.\n";
			continue;
		}
		// check if exists
		auto block = this->browseChild<QUaModbusDataBlock>(strBrowseName);
		if (block)
		{
			strError += QString("Warning : Block with %1 BrowseName already exists. Overwriting Block configuration.\n").arg(strBrowseName);
			// overwrite block config
			// NOTE : loop already should have started
			block->fromDomElement(elemBlock, strError);
			continue;
		}
		block = this->addChild<QUaModbusDataBlock>();
		if (!block)
		{
			strError += QString("Error : Failed to create Block with %1 BrowseName. Skipping.\n").arg(strBrowseName);
			continue;
		}
		block->setDisplayName(strBrowseName);
		block->setBrowseName (strBrowseName);
		// set block config
		block->fromDomElement(elemBlock, strError);
		// start block loop
		block->startLoop();
	}
}
