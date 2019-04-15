#ifndef QUAMODBUSCLIENT_H
#define QUAMODBUSCLIENT_H

#include <QModbusTcpClient>
#include <QModbusRtuSerialMaster>
#include <QSerialPort>

#include <QUaBaseObject>
#include <QUaBaseDataVariable>
#include <QUaProperty>

class QUaModbusClient : public QUaBaseObject
{
    Q_OBJECT

	// UA properties
	Q_PROPERTY(QUaProperty * type READ type)

	// UA variables
	Q_PROPERTY(QUaBaseDataVariable * state READ state)

public:
	Q_INVOKABLE explicit QUaModbusClient(QUaServer *server);

	// UA properties

	QUaProperty * type();

	// UA variables

	QUaBaseDataVariable * state();

	// UA methods

	Q_INVOKABLE QString setType(QString strType);



private:
	QScopedPointer<QModbusClient> m_modbusClient;

};

#endif // QUAMODBUSCLIENT_H

