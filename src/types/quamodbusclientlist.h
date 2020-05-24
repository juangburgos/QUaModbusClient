#ifndef QUAMODBUSCLIENTLIST_H
#define QUAMODBUSCLIENTLIST_H

#ifndef QUA_ACCESS_CONTROL
#include <QUaFolderObject>
#else
#include <QUaFolderObjectProtected>
class QUaPermissionsList;
#endif // !QUA_ACCESS_CONTROL

#include <QDomDocument>
#include <QDomElement>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

class QUaModbusClient;

#ifndef QUA_ACCESS_CONTROL
class QUaModbusClientList : public QUaFolderObject
#else
class QUaModbusClientList : public QUaFolderObjectProtected
#endif // !QUA_ACCESS_CONTROL
{
    Q_OBJECT

public:
	Q_INVOKABLE explicit QUaModbusClientList(QUaServer *server);

	// UA methods

	Q_INVOKABLE QString addTcpClient(const QUaQualifiedName& clientId);

	Q_INVOKABLE QString addRtuSerialClient(const QUaQualifiedName& clientId);

	Q_INVOKABLE void clear();

	Q_INVOKABLE QString xmlConfig();

	Q_INVOKABLE QString setXmlConfig(QString strXmlConfig);

	// C++ API

	QList<QUaModbusClient*> clients();

	QString csvClients();

	QString csvBlocks();

	QString csvValues();

	QQueue<QUaLog> setCsvClients(QString strCsvClients);

	QQueue<QUaLog> setCsvBlocks(QString strCsvBlocks);

	QQueue<QUaLog> setCsvValues(QString strCsvValues);

	void clearInmediatly();

#ifdef QUA_ACCESS_CONTROL
	QUaPermissionsList * getPermissionsList();
#endif // QUA_ACCESS_CONTROL

	// XML import / export
	QDomElement toDomElement  (QDomDocument & domDoc) const;
	void        fromDomElement(QDomElement  & domElem, QQueue<QUaLog>& errorLogs);

private:
	template<typename T>
	QString addClient(const QUaQualifiedName &clientId);

};

template<typename T>
inline QString QUaModbusClientList::addClient(const QUaQualifiedName& clientId)
{
	auto strClientId = clientId.name();
	// check empty
	if (strClientId.isEmpty())
	{
		return tr("%1 : Client Id argument cannot be empty.").arg("Error");
	}
	// check valid length
	if (strClientId.count() > 40)
	{
		return tr("%1 : Client Id cannot contain more than 40 characters.").arg("Error");
	}
	// check not called Name
	if (strClientId.compare("Name", Qt::CaseSensitive) == 0)
	{
		return tr("%1 : Client Id cannot be 'Name'.").arg("Error");
	}
	// check valid characters
	QRegularExpression rx("^[a-zA-Z0-9_]*$");
	QRegularExpressionMatch match = rx.match(strClientId, 0, QRegularExpression::PartialPreferCompleteMatch);
	if (!match.hasMatch())
	{
		return tr("%1 : Client Id can only contain numbers, letters and underscores /^[a-zA-Z0-9_]*$/.").arg("Error");
	}
	// check if id already exists
	if (this->hasChild(clientId))
	{
		return tr("%1 : Client Id already exists.").arg("Error");
	}
	// create instance
	QUaNodeId nodeId = { 0, QString("modbus.%1").arg(strClientId) };
	auto client = this->addChild<T>(clientId, nodeId);
	if (!client)
	{
		return  tr("%1 : NodeId %2 already exists.").arg("Error").arg(nodeId);
	}
	return "Success";
}

#endif // QUAMODBUSCLIENTLIST_H
