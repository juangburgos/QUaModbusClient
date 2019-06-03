#ifndef QUAMODBUSCLIENTWIDGETEDIT_H
#define QUAMODBUSCLIENTWIDGETEDIT_H

#include <QWidget>
#include <QSerialPort>
#include <QSerialPortInfo>

typedef QSerialPort::Parity   QParity;
typedef QSerialPort::BaudRate QBaudRate;
typedef QSerialPort::DataBits QDataBits;
typedef QSerialPort::StopBits QStopBits;

namespace Ui {
class QUaModbusClientWidgetEdit;
}

class QUaModbusClientWidgetEdit : public QWidget
{
    Q_OBJECT

public:
    explicit QUaModbusClientWidgetEdit(QWidget *parent = nullptr);
    ~QUaModbusClientWidgetEdit();

	enum ClientType {
		Tcp     = 0,
		Serial  = 1,
		Invalid = 2
	};
	Q_ENUM(ClientType)

	bool       isIdEditable() const;
	void       setIdEditable(const bool &idEditable);

	bool       isTypeEditable() const;
	void       setTypeEditable(const bool &typeEditable);

	QString    id() const;
	void       setId(const QString &strId);

	ClientType type() const;
	void       setType(const ClientType &type);

	quint8     deviceAddress() const;
	void       setDeviceAddress(const quint8 &deviceAddress);

	quint16    networkPort() const;
	void       setNetworkPort(const quint16 &networkPort);

	bool       keepConnecting() const;
	void       setKeepConnecting(const bool &keepConnecting);

	QString    ipAddress() const;
	void       setIpAddress(const QString &strIpAddress);

	QString    comPort () const;
	void       setComPort(const QString &strComPort);

	int        comPortKey() const;
	void       setComPortKey(const int &comPortKey);

	QParity    parity  () const;
	void       setParity(const QParity &parity);

	QBaudRate  baudRate() const;
	void       setBaudRate(const QBaudRate &baudRate);

	QDataBits  dataBits() const;
	void       setDataBits(const QDataBits &dataBits);

	QStopBits  stopBits() const;
	void       setStopBits(const QStopBits &stopBits);


private slots:
    void on_comboBoxType_currentIndexChanged(int index);

private:
    Ui::QUaModbusClientWidgetEdit *ui;

	void updateTypeInGui(const ClientType & type);
};

#endif // QUAMODBUSCLIENTWIDGETEDIT_H
