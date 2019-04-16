#ifndef QUAMODBUSCLIENTLIST_H
#define QUAMODBUSCLIENTLIST_H

#include <QUaFolderObject>

class QUaModbusClientList : public QUaFolderObject
{
    Q_OBJECT

public:
	Q_INVOKABLE explicit QUaModbusClientList(QUaServer *server);

	// UA methods

	Q_INVOKABLE QString addTcpClient(QString strClientId);

	Q_INVOKABLE QString addRtuSerialClient(QString strClientId);

private:
	template<typename T>
	QString addClient(QString strClientId);

};

template<typename T>
inline QString QUaModbusClientList::addClient(QString strClientId)
{
	strClientId = strClientId.trimmed();
	// check empty
	if (strClientId.isEmpty())
	{
		return "Error : Client Id argument cannot be empty.";
	}
	// check if id already exists
	if (this->hasChild(strClientId))
	{
		return "Error : Client Id already exists.";
	}
	// create instance
	auto client = this->addChild<T>();
	client->setDisplayName(strClientId);
	client->setBrowseName (strClientId);
	return "Success";
}

#endif // QUAMODBUSCLIENTLIST_H
