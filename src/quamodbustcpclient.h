#ifndef QUAMODBUSTCPCLIENT_H
#define QUAMODBUSTCPCLIENT_H

#include "quamodbusclient.h"

class QUaModbusTcpClient : public QUaModbusClient
{
    Q_OBJECT

	// UA properties
	Q_PROPERTY(QUaProperty * networkAddress  READ networkAddress)
	Q_PROPERTY(QUaProperty * networkPort     READ networkPort   )

public:
	Q_INVOKABLE explicit QUaModbusTcpClient(QUaServer *server);

	// UA properties

	QUaProperty * networkAddress();
	QUaProperty * networkPort();

private slots:
	void on_stateChanged         (const QVariant &value);
	void on_networkAddressChanged(const QVariant &value);
	void on_networkPortChanged   (const QVariant &value);

};

#endif // QUAMODBUSTCPCLIENT_H

