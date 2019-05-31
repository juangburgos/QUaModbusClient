#ifndef QUAMODBUSRTUSERIALCLIENT_H
#define QUAMODBUSRTUSERIALCLIENT_H

#include "quamodbusclient.h"

class QUaModbusClientList;

typedef QSerialPort::Parity   QParity;
typedef QSerialPort::BaudRate QBaudRate;
typedef QSerialPort::DataBits QDataBits;
typedef QSerialPort::StopBits QStopBits;

class QUaModbusRtuSerialClient : public QUaModbusClient
{
	friend class QUaModbusClientList;

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

	QUaProperty * comPort () const;
	QUaProperty * parity  () const;
	QUaProperty * baudRate() const;
	QUaProperty * dataBits() const;
	QUaProperty * stopBits() const;

	static QString ComPorts;
	static QMap<int, QByteArray> EnumComPorts();

	// C++ API (all is read/write)

	QString   getComPort() const;
	void      setComPort(const QString &strComPort);

	int       getComPortKey() const;
	void      setComPortKey(const int &comPort);

	QParity   getParity() const;
	void      setParity(const QParity &parity);

	QBaudRate getBaudRate() const;
	void      setBaudRate(const QBaudRate &baudRate);

	QDataBits getDataBits() const;
	void      setDataBits(const QDataBits &dataBits);

	QStopBits getStopBits() const;
	void      setStopBits(const QStopBits &stopBits);

signals:
	// C++ API
	void comPortChanged (const QString   &strComPort);
	void parityChanged  (const QParity   &parity    );
	void baudRateChanged(const QBaudRate &baudRate  );
	void dataBitsChanged(const QDataBits &dataBits  );
	void stopBitsChanged(const QStopBits &stopBits  );

protected:
	// XML import / export
	QDomElement toDomElement  (QDomDocument & domDoc) const override;
	void        fromDomElement(QDomElement  & domElem, QString &strError) override;

private slots:
	void on_comPortChanged (const QVariant &value);
	void on_parityChanged  (const QVariant &value);
	void on_baudRateChanged(const QVariant &value);
	void on_dataBitsChanged(const QVariant &value);
	void on_stopBitsChanged(const QVariant &value);
	// internal
	void on_stateChanged   (const QModbusDevice::State &state);
};

#endif // QUAMODBUSRTUSERIALCLIENT_H

