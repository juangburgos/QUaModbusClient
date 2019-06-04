#include "quamodbusclientwidgettest.h"
#include "ui_quamodbusclientwidgettest.h"

#include <QUaModbusClientList>
#include <QUaModbusClient>
#include <QUaModbusTcpClient>
#include <QUaModbusRtuSerialClient>
#include <QUaModbusDataBlock>
#include <QUaModbusValue>

#include <QUaModbusClientWidgetEdit>
#include <QUaModbusDataBlockWidgetEdit>
#include <QUaModbusValueWidgetEdit>

#include <QUaModbusDataBlockWidgetStatus>
#include <QUaModbusValueWidgetStatus>

QUaModbusClientWidgetTest::QUaModbusClientWidgetTest(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QUaModbusClientWidgetTest)
{
    ui->setupUi(this);
	m_pWidgetEdit    = nullptr;
	m_pWidgetStatus  = nullptr;
	m_typeModbusCurr = QModbusSelectType::Invalid;
	// hide apply button until some valid object selected
	ui->pushButtonApply->setEnabled(false);
	ui->pushButtonApply->setVisible(false);
	// add list entry point to object's folder
	QUaFolderObject * objsFolder = m_server.objectsFolder();
	auto modCliList = objsFolder->addChild<QUaModbusClientList>();
	modCliList->setDisplayName("ModbusClients");
	modCliList->setBrowseName("ModbusClients");
	// set client list into widget
	ui->widgetModbus->setClientList(modCliList);

	// change widgets
	QObject::connect(ui->widgetModbus, &QUaModbusClientWidget::nodeSelectionChanged, ui->pushButtonApply,
		[this](QUaNode * nodePrev, QModbusSelectType typePrev, QUaNode * nodeCurr, QModbusSelectType typeCurr) 
	{
		Q_UNUSED(typePrev);
		// delete old widget if necessary
		if (m_pWidgetEdit)
		{
			delete m_pWidgetEdit;
			m_pWidgetEdit = nullptr;
		}
		// delete old widget if necessary
		if (m_pWidgetStatus)
		{
			delete m_pWidgetStatus;
			m_pWidgetStatus = nullptr;
		}
		if (!nodePrev && !nodeCurr)
		{
			return;
		}
		// show apply button
		ui->pushButtonApply->setEnabled(true);
		ui->pushButtonApply->setVisible(true);
		// set up widgets for current selection
		switch (typeCurr)
		{
		case QModbusSelectType::QUaModbusClient:
			{
				auto client = dynamic_cast<QUaModbusClient*>(nodeCurr);
				this->bindClientWidgetEdit(client);
			}
			break;
		case QModbusSelectType::QUaModbusDataBlock:
			{
				auto block = dynamic_cast<QUaModbusDataBlock*>(nodeCurr);
				this->bindBlockWidgetEdit  (block);
				this->bindBlockWidgetStatus(block);
			}
			break;
		case QModbusSelectType::QUaModbusValue:
			{
				auto value = dynamic_cast<QUaModbusValue*>(nodeCurr);
				this->bindValueWidgetEdit  (value);
				this->bindValueWidgetStatus(value);
			}
			break;
		default:
			// set widgets to null
			if (m_pWidgetEdit)
			{
				delete m_pWidgetEdit;
			}
			m_pWidgetEdit = nullptr;
			if (m_pWidgetStatus)
			{
				delete m_pWidgetStatus;
			}
			m_pWidgetStatus = nullptr;
			// hide apply button
			ui->pushButtonApply->setEnabled(false);
			ui->pushButtonApply->setVisible(false);
			break;
		}
	});
}

QUaModbusClientWidgetTest::~QUaModbusClientWidgetTest()
{
    delete ui;
}

void QUaModbusClientWidgetTest::bindClientWidgetEdit(QUaModbusClient * client)
{
	// create widget if necessary
	Q_ASSERT(!m_pWidgetEdit);
	auto widget = new QUaModbusClientWidgetEdit(this);
	m_pWidgetEdit = widget;
	ui->verticalLayoutConfig->insertWidget(0, widget);
	Q_CHECK_PTR(widget);
	// bind common
	QObject::connect(client, &QObject::destroyed, widget,
	[this]() {
		if (m_pWidgetEdit)
		{
			delete m_pWidgetEdit;
			m_pWidgetEdit = nullptr;
		}
	});
	// id
	widget->setIdEditable(false);
	widget->setId(client->browseName());
	// type
	widget->setTypeEditable(false);
	widget->setType(client->getType());
	// modbus addess
	widget->setDeviceAddress(client->getServerAddress());
	QObject::connect(client, &QUaModbusClient::serverAddressChanged, widget,
	[widget](const quint8 &serverAddress) {
		widget->setDeviceAddress(serverAddress);
	});
	// keep connecting
	widget->setKeepConnecting(client->getKeepConnecting());
	QObject::connect(client, &QUaModbusClient::keepConnectingChanged, widget,
	[widget](const bool &keepConnecting) {
		widget->setKeepConnecting(keepConnecting);
	});
	// check if tcp or serial
	switch (client->getType())
	{
	case QModbusClientType::Tcp:
		{
			auto cliTcp = dynamic_cast<QUaModbusTcpClient*>(client);
			Q_CHECK_PTR(cliTcp);
			// ip address
			widget->setIpAddress(cliTcp->getNetworkAddress());
			QObject::connect(cliTcp, &QUaModbusTcpClient::networkAddressChanged, widget,
			[widget](const QString &strNetworkAddress) {
				widget->setIpAddress(strNetworkAddress);
			});
			// ip port
			widget->setNetworkPort(cliTcp->getNetworkPort());
			QObject::connect(cliTcp, &QUaModbusTcpClient::networkPortChanged, widget,
			[widget](const quint16 &networkPort) {
				widget->setNetworkPort(networkPort);
			});
			// on apply
			QObject::connect(ui->pushButtonApply, &QPushButton::clicked, widget, 
			[cliTcp, widget]() {
				// common
				cliTcp->setServerAddress (widget->deviceAddress());
				cliTcp->setKeepConnecting(widget->keepConnecting());
				// tcp
				cliTcp->setNetworkAddress(widget->ipAddress());
				cliTcp->setNetworkPort   (widget->networkPort());
			});
		}
		break;
	case QModbusClientType::Serial:
		{
			auto cliSerial = dynamic_cast<QUaModbusRtuSerialClient*>(client);
			Q_CHECK_PTR(cliSerial);
			// com port
			widget->setComPortKey(cliSerial->getComPortKey());
			QObject::connect(cliSerial, &QUaModbusRtuSerialClient::comPortChanged, widget,
			[widget](const QString &strComPort) {
				widget->setComPort(strComPort);
			});
			// parity
			widget->setParity(cliSerial->getParity());
			QObject::connect(cliSerial, &QUaModbusRtuSerialClient::parityChanged, widget,
			[widget](const QParity &parity) {
				widget->setParity(parity);
			});
			// baud rate
			widget->setBaudRate(cliSerial->getBaudRate());
			QObject::connect(cliSerial, &QUaModbusRtuSerialClient::baudRateChanged, widget,
			[widget](const QBaudRate &baudRate) {
				widget->setBaudRate(baudRate);
			});
			// data bits
			widget->setDataBits(cliSerial->getDataBits());
			QObject::connect(cliSerial, &QUaModbusRtuSerialClient::dataBitsChanged, widget,
			[widget](const QDataBits &dataBits) {
				widget->setDataBits(dataBits);
			});
			// stop bits
			widget->setStopBits(cliSerial->getStopBits());
			QObject::connect(cliSerial, &QUaModbusRtuSerialClient::stopBitsChanged, widget,
			[widget](const QStopBits &stopBits) {
				widget->setStopBits(stopBits);
			});
			// on apply
			QObject::connect(ui->pushButtonApply, &QPushButton::clicked, widget,
				[cliSerial, widget]() {
				// common
				cliSerial->setServerAddress (widget->deviceAddress());
				cliSerial->setKeepConnecting(widget->keepConnecting());
				// serial
				cliSerial->setComPortKey(widget->comPortKey());
				cliSerial->setParity    (widget->parity());
				cliSerial->setBaudRate  (widget->baudRate());
				cliSerial->setDataBits  (widget->dataBits());
				cliSerial->setStopBits  (widget->stopBits());
			});
		}
		break;
	default:
		Q_ASSERT(false);
		break;
	}
}

void QUaModbusClientWidgetTest::bindBlockWidgetEdit(QUaModbusDataBlock * block)
{
	// create widget if necessary
	Q_ASSERT(!m_pWidgetEdit);
	auto widget = new QUaModbusDataBlockWidgetEdit(this);
	m_pWidgetEdit = widget;
	ui->verticalLayoutConfig->insertWidget(0, widget);
	Q_CHECK_PTR(widget);
	// bind
	QObject::connect(block, &QObject::destroyed, widget,
		[this]() {
		if (m_pWidgetEdit)
		{
			delete m_pWidgetEdit;
			m_pWidgetEdit = nullptr;
		}
	});
	// id
	widget->setIdEditable(false);
	widget->setId(block->browseName());
	// type
	widget->setType(block->getType());
	QObject::connect(block, &QUaModbusDataBlock::typeChanged, widget,
	[widget](const QModbusDataBlockType &type) {
		widget->setType(type);
	});
	// address
	widget->setAddress(block->getAddress());
	QObject::connect(block, &QUaModbusDataBlock::addressChanged, widget,
	[widget](const int &address) {
		widget->setAddress(address);
	});
	// size
	widget->setSize(block->getSize());
	QObject::connect(block, &QUaModbusDataBlock::sizeChanged, widget,
	[widget](const quint32 &size) {
		widget->setSize(size);
	});
	// sampling
	widget->setSamplingTime(block->getSamplingTime());
	QObject::connect(block, &QUaModbusDataBlock::samplingTimeChanged, widget,
	[widget](const quint32 &samplingTime) {
		widget->setSamplingTime(samplingTime);
	});
	// on apply
	QObject::connect(ui->pushButtonApply, &QPushButton::clicked, widget,
	[block, widget]() {
		block->setType        (widget->type()        );
		block->setAddress     (widget->address()     );
		block->setSize        (widget->size()        );
		block->setSamplingTime(widget->samplingTime());
	});
}

void QUaModbusClientWidgetTest::bindValueWidgetEdit(QUaModbusValue * value)
{
	// create widget if necessary
	Q_ASSERT(!m_pWidgetEdit);
	auto widget = new QUaModbusValueWidgetEdit(this);
	m_pWidgetEdit = widget;
	ui->verticalLayoutConfig->insertWidget(0, widget);
	Q_CHECK_PTR(widget);
	// bind
	QObject::connect(value, &QObject::destroyed, widget,
		[this]() {
		if (m_pWidgetEdit)
		{
			delete m_pWidgetEdit;
			m_pWidgetEdit = nullptr;
		}
	});
	// id
	widget->setIdEditable(false);
	widget->setId(value->browseName());
	// type
	widget->setType(value->getType());
	QObject::connect(value, &QUaModbusValue::typeChanged, widget,
	[widget](const QModbusValueType &type) {
		widget->setType(type);
	});
	// offset
	widget->setOffset(value->getAddressOffset());
	QObject::connect(value, &QUaModbusValue::addressOffsetChanged, widget,
	[widget](const int &addressOffset) {
		widget->setOffset(addressOffset);
	});
	// on apply
	QObject::connect(ui->pushButtonApply, &QPushButton::clicked, widget,
	[value, widget]() {
		value->setType         (widget->type()  );
		value->setAddressOffset(widget->offset());
	});
}

void QUaModbusClientWidgetTest::bindBlockWidgetStatus(QUaModbusDataBlock * block)
{
	// create widget if necessary
	Q_ASSERT(!m_pWidgetStatus);
	auto widget = new QUaModbusDataBlockWidgetStatus(this);
	m_pWidgetStatus = widget;
	ui->verticalLayoutStatus->insertWidget(0, widget);
	Q_CHECK_PTR(widget);
	// bind
	QObject::connect(block, &QObject::destroyed, widget,
		[this]() {
		if (m_pWidgetStatus)
		{
			delete m_pWidgetStatus;
			m_pWidgetStatus = nullptr;
		}
	});
	// status
	widget->setStatus(block->getLastError());
	QObject::connect(block, &QUaModbusDataBlock::lastErrorChanged, widget,
	[widget](const QModbusError & error) {
		widget->setStatus(error);
	});
	// data
	widget->setData(0, block->getData());
	QObject::connect(block, &QUaModbusDataBlock::dataChanged, widget,
	[widget](const QVector<quint16> & data) {
		widget->setData(0, data);
	});
}

void QUaModbusClientWidgetTest::bindValueWidgetStatus(QUaModbusValue * value)
{
	// create widget if necessary
	Q_ASSERT(!m_pWidgetStatus);
	auto widget = new QUaModbusValueWidgetStatus(this);
	m_pWidgetStatus = widget;
	ui->verticalLayoutStatus->insertWidget(0, widget);
	Q_CHECK_PTR(widget);
	// bind
	QObject::connect(value, &QObject::destroyed, widget,
		[this]() {
		if (m_pWidgetStatus)
		{
			delete m_pWidgetStatus;
			m_pWidgetStatus = nullptr;
		}
	});
	// type
	widget->setType(value->getType());
	QObject::connect(value, &QUaModbusValue::typeChanged, widget,
	[widget, value](const QModbusValueType & type) {
		widget->setType(type);
		widget->setValue(value->getValue());
	});
	// status
	widget->setStatus(value->getLastError());
	QObject::connect(value, &QUaModbusValue::lastErrorChanged, widget,
	[widget](const QModbusError & error) {
		widget->setStatus(error);
	});
	// registers used
	widget->setRegistersUsed(value->getRegistersUsed());
	QObject::connect(value, &QUaModbusValue::registersUsedChanged, widget,
	[widget](const quint16 & registersUsed) {
		widget->setRegistersUsed(registersUsed);
	});
	// data & value
	auto data   = value->block()->getData();
	auto offset = value->getAddressOffset();
	auto size   = QUaModbusValue::typeBlockSize(value->getType());
	widget->setData(data.mid(offset, size));
	widget->setValue(value->getValue());
	QObject::connect(value, &QUaModbusValue::valueChanged, widget,
	[widget, value](const QVariant & varVal) {
		auto block  = value->block()->getData();
		auto offset = value->getAddressOffset();
		auto size   = QUaModbusValue::typeBlockSize(value->getType());
		widget->setData(block.mid(offset, size));
		widget->setValue(varVal);
	});
	// writable
	auto blkType = value->block()->getType();
	widget->setWritable(blkType == QModbusDataBlockType::Coils || blkType == QModbusDataBlockType::HoldingRegisters);
	QObject::connect(value->block(), &QUaModbusDataBlock::typeChanged, widget,
	[widget](const QModbusDataBlockType &type) {
		widget->setWritable(type == QModbusDataBlockType::Coils || type == QModbusDataBlockType::HoldingRegisters);
	});
	// on value change
	QObject::connect(widget, &QUaModbusValueWidgetStatus::valueUpdated, value,
	[value](const QVariant &varVal) {
		value->setValue(varVal);
	});
}

void QUaModbusClientWidgetTest::on_pushButtonStart_clicked()
{
	m_server.isRunning() ?
		ui->pushButtonStart->setText(tr("Start")) :
		ui->pushButtonStart->setText(tr("Stop"));
	m_server.isRunning() ?
		m_server.stop() :
		m_server.start();
}
