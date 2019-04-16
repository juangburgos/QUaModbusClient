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

	// TODO : add strBlockId and make it displayname/browsename, make sure unique, return QString
	Q_INVOKABLE void addDataBlock(const quint16 &uiModiconStartAddress, const quint16 &uiLength);


private:
	QUaModbusClient * client();

};

#endif // QUAMODBUSDATABLOCKLIST_H


