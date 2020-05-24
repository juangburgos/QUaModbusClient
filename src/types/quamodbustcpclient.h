#ifndef QUAMODBUSTCPCLIENT_H
#define QUAMODBUSTCPCLIENT_H

#include "quamodbusclient.h"

class QUaModbusClientList;

class QUaModbusTcpClient : public QUaModbusClient
{
	friend class QUaModbusClientList;

    Q_OBJECT

	// UA properties
	Q_PROPERTY(QUaProperty * NetworkAddress  READ networkAddress)
	Q_PROPERTY(QUaProperty * NetworkPort     READ networkPort   )

public:
	Q_INVOKABLE explicit QUaModbusTcpClient(QUaServer *server);

	// UA properties

	QUaProperty * networkAddress() const;
	QUaProperty * networkPort() const;

	// C++ API (all is read/write)

	QString  getNetworkAddress() const;
	void     setNetworkAddress(const QString &strNetworkAddress);

	quint16  getNetworkPort() const;
	void     setNetworkPort(const quint16 &networkPort);

signals:
	// C++ API
	void networkAddressChanged(const QString &strNetworkAddress);
	void networkPortChanged(const quint16 &networkPort);

protected:
	// XML import / export
	QDomElement toDomElement  (QDomDocument & domDoc) const override;
	void        fromDomElement(QDomElement  & domElem, QQueue<QUaLog>& errorLogs) override;

private slots:
	void on_stateChanged         (const QModbusDevice::State &state);
	void on_networkAddressChanged(const QVariant &value);
	void on_networkPortChanged   (const QVariant &value);

};

#endif // QUAMODBUSTCPCLIENT_H

