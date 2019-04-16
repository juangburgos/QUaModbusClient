#ifndef QUAMODBUSRTUSERIALCLIENT_H
#define QUAMODBUSRTUSERIALCLIENT_H

#include "quamodbusclient.h"

class QUaModbusRtuSerialClient : public QUaModbusClient
{
    Q_OBJECT

	// UA properties
	Q_PROPERTY(QUaProperty * ComPort  READ comPort )
	Q_PROPERTY(QUaProperty * Parity   READ parity  )
	Q_PROPERTY(QUaProperty * BaudRate READ baudRate)
	Q_PROPERTY(QUaProperty * DataBits READ dataBits)
	Q_PROPERTY(QUaProperty * StopBits READ stopBits)

public:
	Q_INVOKABLE explicit QUaModbusRtuSerialClient(QUaServer *server);

	// UA properties

	QUaProperty * comPort ();
	QUaProperty * parity  ();
	QUaProperty * baudRate();
	QUaProperty * dataBits();
	QUaProperty * stopBits();

private slots:
	void on_stateChanged   (const QVariant &value);
	void on_comPortChanged (const QVariant &value);
	void on_parityChanged  (const QVariant &value);
	void on_baudRateChanged(const QVariant &value);
	void on_dataBitsChanged(const QVariant &value);
	void on_stopBitsChanged(const QVariant &value);
};

#endif // QUAMODBUSRTUSERIALCLIENT_H

