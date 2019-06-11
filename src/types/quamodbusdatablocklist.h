#ifndef QUAMODBUSDATABLOCKLIST_H
#define QUAMODBUSDATABLOCKLIST_H

#include <QUaFolderObject>

#include <QDomDocument>
#include <QDomElement>

class QUaModbusClient;
class QUaModbusTcpClient;
class QUaModbusRtuSerialClient;
class QUaModbusDataBlock;

class QUaModbusDataBlockList : public QUaFolderObject
{
	friend class QUaModbusClient;
	friend class QUaModbusTcpClient;
	friend class QUaModbusRtuSerialClient;
	friend class QUaModbusDataBlock;

    Q_OBJECT

public:
	Q_INVOKABLE explicit QUaModbusDataBlockList(QUaServer *server);

	// UA methods

	Q_INVOKABLE QString addDataBlock(QString strBlockId);

	Q_INVOKABLE void clear();

	// C++ API

	QList<QUaModbusDataBlock*> blocks();

private:
	QUaModbusClient * client();

	// XML import / export
	QDomElement toDomElement  (QDomDocument & domDoc) const;
	void        fromDomElement(QDomElement  & domElem, QString &strError);

};

#endif // QUAMODBUSDATABLOCKLIST_H


