#ifndef QUAMODBUSCLIENT_H
#define QUAMODBUSCLIENT_H

#include <QModbusTcpClient>
#include <QModbusRtuSerialMaster>
#include <QSerialPort>

#include <QLambdaThreadWorker>

#include <QUaBaseObject>
#include <QUaBaseDataVariable>
#include <QUaProperty>

#include <QDomDocument>
#include <QDomElement>

#include "quamodbusdatablocklist.h"

class QUaModbusClientList;
class QUaModbusDataBlock;

class QUaModbusClient : public QUaBaseObject
{
	friend class QUaModbusClientList;
	friend class QUaModbusDataBlockList;
	friend class QUaModbusDataBlock;

    Q_OBJECT

	// UA properties
	Q_PROPERTY(QUaProperty * Type           READ type          )
	Q_PROPERTY(QUaProperty * ServerAddress  READ serverAddress )
	Q_PROPERTY(QUaProperty * KeepConnecting READ keepConnecting)

	// UA variables
	Q_PROPERTY(QUaBaseDataVariable * State     READ state    )
	Q_PROPERTY(QUaBaseDataVariable * LastError READ lastError)

	// UA objects
	Q_PROPERTY(QUaModbusDataBlockList * DataBlocks READ dataBlocks)

public:
	Q_INVOKABLE explicit QUaModbusClient(QUaServer *server);

	// UA properties

	QUaProperty * type() const;
	QUaProperty * serverAddress() const;
	QUaProperty * keepConnecting() const;

	// UA variables

	QUaBaseDataVariable * state() const;
	QUaBaseDataVariable * lastError() const;

	// UA objects

	QUaModbusDataBlockList * dataBlocks() const;

	// UA methods

	Q_INVOKABLE void remove();
	Q_INVOKABLE void connectDevice();
	Q_INVOKABLE void disconnectDevice();

signals:
	void stateChanged(QModbusDevice::State state);

protected:
	QLambdaThreadWorker           m_workerThread;
	QSharedPointer<QModbusClient> m_modbusClient;

	QModbusDevice::State getState();
	void setupModbusClient();

	// XML import / export
	virtual QDomElement toDomElement  (QDomDocument & domDoc) const;
	virtual void        fromDomElement(QDomElement  & domElem, QString &strError);

private slots:
	void on_stateChanged(QModbusDevice::State state);
	void on_errorChanged(QModbusDevice::Error error);
};

#endif // QUAMODBUSCLIENT_H

