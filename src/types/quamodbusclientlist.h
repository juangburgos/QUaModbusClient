#ifndef QUAMODBUSCLIENTLIST_H
#define QUAMODBUSCLIENTLIST_H

#include <QUaFolderObject>

#include <QDomDocument>
#include <QDomElement>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

class QUaModbusClientList : public QUaFolderObject
{
    Q_OBJECT

public:
	Q_INVOKABLE explicit QUaModbusClientList(QUaServer *server);

	// UA methods

	Q_INVOKABLE QString addTcpClient(QString strClientId);

	Q_INVOKABLE QString addRtuSerialClient(QString strClientId);

	Q_INVOKABLE QString xmlConfig();

	Q_INVOKABLE QString setXmlConfig(QString strXmlConfig);

private:
	template<typename T>
	QString addClient(QString strClientId);

	// XML import / export
	QDomElement toDomElement  (QDomDocument & domDoc) const;
	void        fromDomElement(QDomElement  & domElem, QString &strError);

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
	// TODO : set custom nodeId when https://github.com/open62541/open62541/issues/2667 fixed
	//QString strNodeId = QString("ns=1;s=%1").arg(this->nodeBrowsePath().join(".") + "." + strClientId);
	auto client = this->addChild<T>(/*strNodeId*/);
	client->setDisplayName(strClientId);
	client->setBrowseName (strClientId);
	return "Success";
}

#endif // QUAMODBUSCLIENTLIST_H
