#include "quamodbusrtuserialclient.h"

QUaModbusRtuSerialClient::QUaModbusRtuSerialClient(QUaServer *server)
	: QUaModbusClient(server)
{
	// set defaults
	type    ()->setValue("Serial");
	comPort ()->setValue("COM0");
	parity  ()->setDataTypeEnum(QMetaEnum::fromType<QSerialPort::Parity>());
	parity  ()->setValue(QSerialPort::EvenParity);
	baudRate()->setDataTypeEnum(QMetaEnum::fromType<QSerialPort::BaudRate>());
	baudRate()->setValue(QSerialPort::Baud19200);
	dataBits()->setDataTypeEnum(QMetaEnum::fromType<QSerialPort::DataBits>());
	dataBits()->setValue(QSerialPort::Data8);
	stopBits()->setDataTypeEnum(QMetaEnum::fromType<QSerialPort::StopBits>());
	stopBits()->setValue(QSerialPort::OneStop);
	// set initial conditions
	comPort ()->setWriteAccess(true);
	parity  ()->setWriteAccess(true);
	baudRate()->setWriteAccess(true);
	dataBits()->setWriteAccess(true);
	stopBits()->setWriteAccess(true);
	// instantiate client
	m_workerThread.execInThread([this]() {
		// instantiate in thread so it runs on the thread
		m_modbusClient.reset(new QModbusRtuSerialMaster(nullptr));
		// defaults
		m_modbusClient->setConnectionParameter(QModbusDevice::SerialPortNameParameter, "COM0");
		m_modbusClient->setConnectionParameter(QModbusDevice::SerialParityParameter  , QSerialPort::EvenParity);
		m_modbusClient->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, QSerialPort::Baud19200 );
		m_modbusClient->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, QSerialPort::Data8     );
		m_modbusClient->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, QSerialPort::OneStop   );
		// setup client (call base class method)
		this->setupModbusClient();
	});
	// handle state changes
	QObject::connect(state   (), &QUaBaseVariable::valueChanged, this, &QUaModbusRtuSerialClient::on_stateChanged   , Qt::QueuedConnection);
	QObject::connect(comPort (), &QUaBaseVariable::valueChanged, this, &QUaModbusRtuSerialClient::on_comPortChanged , Qt::QueuedConnection);
	QObject::connect(parity  (), &QUaBaseVariable::valueChanged, this, &QUaModbusRtuSerialClient::on_parityChanged  , Qt::QueuedConnection);
	QObject::connect(baudRate(), &QUaBaseVariable::valueChanged, this, &QUaModbusRtuSerialClient::on_baudRateChanged, Qt::QueuedConnection);
	QObject::connect(dataBits(), &QUaBaseVariable::valueChanged, this, &QUaModbusRtuSerialClient::on_dataBitsChanged, Qt::QueuedConnection);
	QObject::connect(stopBits(), &QUaBaseVariable::valueChanged, this, &QUaModbusRtuSerialClient::on_stopBitsChanged, Qt::QueuedConnection);
}

QUaProperty * QUaModbusRtuSerialClient::comPort()
{
	return this->browseChild<QUaProperty>("comPort");
}

QUaProperty * QUaModbusRtuSerialClient::parity()
{
	return this->browseChild<QUaProperty>("parity");
}

QUaProperty * QUaModbusRtuSerialClient::baudRate()
{
	return this->browseChild<QUaProperty>("baudRate");
}

QUaProperty * QUaModbusRtuSerialClient::dataBits()
{
	return this->browseChild<QUaProperty>("dataBits");
}

QUaProperty * QUaModbusRtuSerialClient::stopBits()
{
	return this->browseChild<QUaProperty>("stopBits");
}

void QUaModbusRtuSerialClient::on_stateChanged(const QVariant &value)
{
	QModbusDevice::State state = value.value<QModbusDevice::State>();
	// only allow to write connection params if not connected
	if (state == QModbusDevice::State::UnconnectedState)
	{
		comPort ()->setWriteAccess(true);
		parity  ()->setWriteAccess(true);
		baudRate()->setWriteAccess(true);
		dataBits()->setWriteAccess(true);
		stopBits()->setWriteAccess(true);
	}
	else
	{
		comPort ()->setWriteAccess(false);
		parity  ()->setWriteAccess(false);
		baudRate()->setWriteAccess(false);
		dataBits()->setWriteAccess(false);
		stopBits()->setWriteAccess(false);
	}
}

void QUaModbusRtuSerialClient::on_comPortChanged(const QVariant & value)
{
	Q_ASSERT_X(this->getState() == QModbusDevice::State::UnconnectedState,
		"QUaModbusTcpClient::on_comPortChanged",
		"Cannot change com port while connected.");
	QString strComPort = value.toString();
	// set in thread, for thread-safety
	m_workerThread.execInThread([this, strComPort]() {
		m_modbusClient->setConnectionParameter(QModbusDevice::SerialPortNameParameter, strComPort);
	});
}

void QUaModbusRtuSerialClient::on_parityChanged(const QVariant & value)
{
	Q_ASSERT_X(this->getState() == QModbusDevice::State::UnconnectedState,
		"QUaModbusTcpClient::on_parityChanged",
		"Cannot change parity while connected.");
	QSerialPort::Parity parity = value.value<QSerialPort::Parity>();
	// set in thread, for thread-safety
	m_workerThread.execInThread([this, parity]() {
		m_modbusClient->setConnectionParameter(QModbusDevice::SerialParityParameter, parity);
	});
}

void QUaModbusRtuSerialClient::on_baudRateChanged(const QVariant & value)
{
	Q_ASSERT_X(this->getState() == QModbusDevice::State::UnconnectedState,
		"QUaModbusTcpClient::on_baudRateChanged",
		"Cannot change baud rate while connected.");
	QSerialPort::BaudRate baudRate = value.value<QSerialPort::BaudRate>();
	// set in thread, for thread-safety
	m_workerThread.execInThread([this, baudRate]() {
		m_modbusClient->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, baudRate);
	});
}

void QUaModbusRtuSerialClient::on_dataBitsChanged(const QVariant & value)
{
	Q_ASSERT_X(this->getState() == QModbusDevice::State::UnconnectedState,
		"QUaModbusTcpClient::on_dataBitsChanged",
		"Cannot change data bits while connected.");
	QSerialPort::DataBits dataBits = value.value<QSerialPort::DataBits>();
	// set in thread, for thread-safety
	m_workerThread.execInThread([this, dataBits]() {
		m_modbusClient->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, dataBits);
	});
}

void QUaModbusRtuSerialClient::on_stopBitsChanged(const QVariant & value)
{
	Q_ASSERT_X(this->getState() == QModbusDevice::State::UnconnectedState,
		"QUaModbusTcpClient::on_stopBitsChanged",
		"Cannot change stop bits while connected.");
	QSerialPort::StopBits stopBits = value.value<QSerialPort::StopBits>();
	// set in thread, for thread-safety
	m_workerThread.execInThread([this, stopBits]() {
		m_modbusClient->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, stopBits);
	});
}