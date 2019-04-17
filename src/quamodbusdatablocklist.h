#ifndef QUAMODBUSDATABLOCKLIST_H
#define QUAMODBUSDATABLOCKLIST_H

#include <QUaFolderObject>

class QUaModbusClient;
class QUaModbusDataBlock;

class QUaModbusDataBlockList : public QUaFolderObject
{
	friend class QUaModbusDataBlock;

    Q_OBJECT

public:
	Q_INVOKABLE explicit QUaModbusDataBlockList(QUaServer *server);

	// UA methods

	Q_INVOKABLE QString addDataBlock(QString strBlockId);


private:
	QUaModbusClient * client();

};

#endif // QUAMODBUSDATABLOCKLIST_H


