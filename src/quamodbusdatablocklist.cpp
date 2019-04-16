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

//auto client = this->client();
//qDebug() << client->m_workerThread.getThreadId();
//qDebug() << client->m_modbusClient->state();

void QUaModbusDataBlockList::addDataBlock(const quint16 &uiModiconStartAddress, const quint16 &uiLength)
{
	QUaModbusDataBlock * block = this->addChild<QUaModbusDataBlock>();
	block->modiconStartAddress()->setValue(uiModiconStartAddress);
	block->length()->setValue(uiLength);
}

QUaModbusClient * QUaModbusDataBlockList::client()
{
	return dynamic_cast<QUaModbusClient*>(this->parent());
}
