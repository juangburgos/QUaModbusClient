#ifndef QUAMODBUSCLIENT_H
#define QUAMODBUSCLIENT_H

#include <QModbusTcpClient>
#include <QModbusRtuSerialMaster>
#include <QSerialPort>

#include <QLambdaThreadWorker>

#include <QUaBaseObject>
#include <QUaBaseDataVariable>
#include <QUaProperty>

class QUaModbusClient : public QUaBaseObject
{
    Q_OBJECT

	// UA properties
	Q_PROPERTY(QUaProperty * type READ type)

	// UA variables
	Q_PROPERTY(QUaBaseDataVariable * state     READ state    )
	Q_PROPERTY(QUaBaseDataVariable * lastError READ lastError)

public:
	Q_INVOKABLE explicit QUaModbusClient(QUaServer *server);

	// UA properties

	QUaProperty * type();

	// UA variables

	QUaBaseDataVariable * state();
	QUaBaseDataVariable * lastError();

	// UA methods

	Q_INVOKABLE void remove();
	Q_INVOKABLE void connectDevice();
	Q_INVOKABLE void disconnectDevice();


protected:
	QLambdaThreadWorker           m_workerThread;
	QScopedPointer<QModbusClient> m_modbusClient;

	QModbusDevice::State getState();
	void setupModbusClient();

private slots:
	void on_stateChanged(QModbusDevice::State state);
	void on_errorChanged(QModbusDevice::Error error);

};

#endif // QUAMODBUSCLIENT_H

