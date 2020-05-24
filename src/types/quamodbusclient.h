#ifndef QUAMODBUSCLIENT_H
#define QUAMODBUSCLIENT_H

#include <QModbusClient>
#include <QModbusTcpClient>
#include <QModbusRtuSerialMaster>
#include <QSerialPort>
#include <QMutex>
#include <QSharedPointer>

#include <QLambdaThreadWorker>

#ifndef QUA_ACCESS_CONTROL
#include <QUaBaseObject>
#else
#include <QUaBaseObjectProtected>
#endif // !QUA_ACCESS_CONTROL

#include <QUaBaseDataVariable>
#include <QUaProperty>

#include <QDomDocument>
#include <QDomElement>

#include "quamodbusdatablocklist.h"

class QUaModbusClientList;
class QUaModbusDataBlock;

typedef QModbusDevice::State QModbusState;
typedef QModbusDevice::Error QModbusError;

#ifndef QUA_ACCESS_CONTROL
class QUaModbusClient : public QUaBaseObject
#else
class QUaModbusClient : public QUaBaseObjectProtected
#endif // !QUA_ACCESS_CONTROL
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

	enum ClientType {
		Tcp     = 0,
		Serial  = 1,
		Invalid = 2
	};
	Q_ENUM(ClientType)
	typedef QUaModbusClient::ClientType QModbusClientType;

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

	// C++ API

	QModbusClientType getType() const;

	QModbusState getState() const;
	void         setState(const QModbusState &state);

	quint8 getServerAddress() const;
	void   setServerAddress(const quint8 &serverAddress);

	bool   getKeepConnecting() const;
	void   setKeepConnecting(const bool &keepConnecting);

	QModbusError getLastError() const;
	void         setLastError(const QModbusError &error);

	QUaModbusClientList * list() const;

signals:
	// C++ API
	void serverAddressChanged (const quint8 &serverAddress);
	void keepConnectingChanged(const bool   &keepConnecting);
	void stateChanged    (const QModbusState &state);
	void lastErrorChanged(const QModbusError &error);

protected:
	QLambdaThreadWorker           m_workerThread;
	QSharedPointer<QModbusClient> m_modbusClient;

	void setupModbusClient();

	// XML import / export
	virtual QDomElement toDomElement  (QDomDocument & domDoc) const;
	virtual void        fromDomElement(QDomElement  & domElem, QQueue<QUaLog>& errorLogs);

private slots:
	void on_serverAddressChanged (const QVariant & value);
	void on_keepConnectingChanged(const QVariant & value);
	void on_stateChanged(QModbusState state);
	void on_errorChanged(QModbusError error);

private:
	QMutex m_mutex;
};

typedef QUaModbusClient::ClientType QModbusClientType;

#endif // QUAMODBUSCLIENT_H

