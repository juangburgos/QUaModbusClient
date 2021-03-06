#ifndef QUAMODBUSDATABLOCKLIST_H
#define QUAMODBUSDATABLOCKLIST_H

#ifndef QUA_ACCESS_CONTROL
#include <QUaFolderObject>
#else
#include <QUaFolderObjectProtected>
#endif // !QUA_ACCESS_CONTROL

#include <QDomDocument>
#include <QDomElement>

class QUaModbusClient;
class QUaModbusTcpClient;
class QUaModbusRtuSerialClient;
class QUaModbusDataBlock;

#ifndef QUA_ACCESS_CONTROL
class QUaModbusDataBlockList : public QUaFolderObject
#else
class QUaModbusDataBlockList : public QUaFolderObjectProtected
#endif // !QUA_ACCESS_CONTROL
{
	friend class QUaModbusClient;
	friend class QUaModbusTcpClient;
	friend class QUaModbusRtuSerialClient;
	friend class QUaModbusDataBlock;

    Q_OBJECT

public:
	Q_INVOKABLE explicit QUaModbusDataBlockList(QUaServer *server);

	// UA methods

	Q_INVOKABLE QString addDataBlock(const QUaQualifiedName& blockId);

	Q_INVOKABLE void clear();

	// C++ API

	QList<QUaModbusDataBlock*> blocks();

signals:
	void aboutToClear();

private:
	QUaModbusClient * client();

	// XML import / export
	QDomElement toDomElement  (QDomDocument & domDoc) const;
	void        fromDomElement(QDomElement  & domElem, QQueue<QUaLog>& errorLogs);

};

#endif // QUAMODBUSDATABLOCKLIST_H


