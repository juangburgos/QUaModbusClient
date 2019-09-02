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

	Q_INVOKABLE QString addTcpClient(QString strClientId);

	Q_INVOKABLE QString addRtuSerialClient(QString strClientId);

	Q_INVOKABLE void clear();

	Q_INVOKABLE QString xmlConfig();

	Q_INVOKABLE QString setXmlConfig(QString strXmlConfig);

	Q_INVOKABLE QString csvClients();

	Q_INVOKABLE QString csvBlocks();

	Q_INVOKABLE QString csvValues();

	Q_INVOKABLE QString setCsvClients(QString strCsvClients);

	Q_INVOKABLE QString setCsvBlocks(QString strCsvBlocks);

	Q_INVOKABLE QString setCsvValues(QString strCsvValues);

	// C++ API

	QList<QUaModbusClient*> clients();

	void clearInmediatly();

#ifdef QUA_ACCESS_CONTROL
	QUaPermissionsList * getPermissionsList();
#endif // QUA_ACCESS_CONTROL

	// XML import / export
	QDomElement toDomElement  (QDomDocument & domDoc) const;
	void        fromDomElement(QDomElement  & domElem, QString &strError);

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
		return tr("%1 : Client Id argument cannot be empty.").arg("Error");
	}
	// check valid length
	if (strClientId.count() > 6)
	{
		return tr("%1 : Client Id cannot contain more than 6 characters.").arg("Error");
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
	if (this->hasChild(strClientId))
	{
		return tr("%1 : Client Id already exists.").arg("Error");
	}
	// create instance
	QString strNodeId = QString("ns=1;s=modbus.%1").arg(strClientId);
	auto client = this->addChild<T>(strNodeId);
	if (!client)
	{
		return  tr("%1 : NodeId %2 already exists.").arg("Error").arg(strNodeId);
	}
	client->setDisplayName(strClientId);
	client->setBrowseName (strClientId);
	return "Success";
}

#endif // QUAMODBUSCLIENTLIST_H
