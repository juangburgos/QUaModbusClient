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
	friend class QUaModbusValue;

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
	~QUaModbusClient();

	enum ClientType {
		Tcp     = 0,
		Serial  = 1,
		Invalid = 2
	};
	Q_ENUM(ClientType)
	typedef QUaModbusClient::ClientType QModbusClientType;

	// UA properties

	QUaProperty * type();
	QUaProperty * serverAddress();
	QUaProperty * keepConnecting();

	// UA variables

	QUaBaseDataVariable * state();
	QUaBaseDataVariable * lastError();

	// UA objects

	QUaModbusDataBlockList * dataBlocks();

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

    // Fix for GCC : cannot be protected or "virtual is protected within this context" error
    virtual void resetModbusClient();

signals:
	// C++ API
	void serverAddressChanged (const quint8 &serverAddress );
	void keepConnectingChanged(const bool   &keepConnecting);
	void stateChanged    (const QModbusState &state);
	void lastErrorChanged(const QModbusError &error);
	void aboutToDestroy();

protected:
	QMutex m_mutex;
	QLambdaThreadWorker           m_workerThread;
	QSharedPointer<QModbusClient> m_modbusClient;

	// XML import / export
	// NOTE : cannot be pure virtual, else moc fails
	virtual QDomElement toDomElement  (QDomDocument & domDoc) const;
	virtual void        fromDomElement(QDomElement  & domElem, QQueue<QUaLog>& errorLogs);

private slots:
	void on_serverAddressChanged (const QVariant & value, const bool& networkChange);
	void on_keepConnectingChanged(const QVariant & value, const bool& networkChange);
	void on_stateChanged(QModbusState state);
	void on_errorChanged(QModbusError error);

private:
	bool m_disconnectRequested;
	QUaProperty* m_type;
	QUaProperty* m_serverAddress;
	QUaProperty* m_keepConnecting;
	QUaBaseDataVariable* m_state;
	QUaBaseDataVariable* m_lastError;
	QUaModbusDataBlockList* m_dataBlocks;
};

typedef QUaModbusClient::ClientType QModbusClientType;

#endif // QUAMODBUSCLIENT_H

