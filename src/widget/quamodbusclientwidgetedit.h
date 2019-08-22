#ifndef QUAMODBUSCLIENTWIDGETEDIT_H
#define QUAMODBUSCLIENTWIDGETEDIT_H

#include <QWidget>
#include <QUaModbusClient>

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

	bool       isIdEditable() const;
	void       setIdEditable(const bool &idEditable);

	bool       isTypeEditable() const;
	void       setTypeEditable(const bool &typeEditable);

	bool       isDeviceAddressEditable() const;
	void       setDeviceAddressEditable(const bool &deviceAddressEditable);

	bool       isKeepConnectingEditable() const;
	void       setKeepConnectingEditable(const bool &keepConnectingEditable);

	bool       isIpAddressEditable() const;
	void       setIpAddressEditable(const bool &ipAddressEditable);

	bool       isNetworkPortEditable() const;
	void       setNetworkPortEditable(const bool &networkPortEditable);

	bool       isComPortEditable() const;
	void       setComPortEditable(const bool &comPortEditable);

	bool       isParityEditable() const;
	void       setParityEditable(const bool &parityEditable);

	bool       isBaudRateEditable() const;
	void       setBaudRateEditable(const bool &baudRateEditable);

	bool       isDataBitsEditable() const;
	void       setDataBitsEditable(const bool &dataBitsEditable);

	bool       isStopBitsEditable() const;
	void       setStopBitsEditable(const bool &stopBitsEditable);

	QString    id() const;
	void       setId(const QString &strId);

	QModbusClientType type() const;
	void              setType(const QModbusClientType &type);

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

    void on_checkBoxKeepConnect_toggled(bool checked);

private:
    Ui::QUaModbusClientWidgetEdit *ui;

	void updateTypeInGui(const QModbusClientType & type);
};

#endif // QUAMODBUSCLIENTWIDGETEDIT_H
