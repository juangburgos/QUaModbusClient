#ifndef QUAMODBUSCLIENT_H
#define QUAMODBUSCLIENT_H

#include <QModbusTcpClient>
#include <QModbusRtuSerialMaster>
#include <QSerialPort>

#include <QLambdaThreadWorker>

#include <QUaBaseObject>
#include <QUaBaseDataVariable>
#include <QUaProperty>

#include "quamodbusdatablocklist.h"

class QUaModbusClient : public QUaBaseObject
{
	friend class QUaModbusDataBlockList;

    Q_OBJECT

	// UA properties
	Q_PROPERTY(QUaProperty * Type READ type)

	// UA variables
	Q_PROPERTY(QUaBaseDataVariable * State     READ state    )
	Q_PROPERTY(QUaBaseDataVariable * LastError READ lastError)

	// UA objects
	Q_PROPERTY(QUaModbusDataBlockList * DataBlocks READ dataBlocks)

public:
	Q_INVOKABLE explicit QUaModbusClient(QUaServer *server);

	// UA properties

	QUaProperty * type();

	// UA variables

	QUaBaseDataVariable * state();
	QUaBaseDataVariable * lastError();

	// UA objects

	QUaModbusDataBlockList * dataBlocks();

	// UA methods

	Q_INVOKABLE void remove();
	Q_INVOKABLE void connectDevice();
	Q_INVOKABLE void disconnectDevice();


protected:
	QLambdaThreadWorker           m_workerThread;
	QSharedPointer<QModbusClient> m_modbusClient;

	QModbusDevice::State getState();
	void setupModbusClient();

private slots:
	void on_stateChanged(QModbusDevice::State state);
	void on_errorChanged(QModbusDevice::Error error);

};

#endif // QUAMODBUSCLIENT_H

