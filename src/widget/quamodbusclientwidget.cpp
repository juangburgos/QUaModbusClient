#include "quamodbusclientwidget.h"
#include "ui_quamodbusclientwidget.h"

#include <QMessageBox>

#include <QUaModbusTcpClient>
#include <QUaModbusRtuSerialClient>

#include <QUaModbusClientDialog>
#include <QUaModbusDataBlockWidgetEdit>

QUaModbusClientWidget::QUaModbusClientWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QUaModbusClientWidget)
{
    ui->setupUi(this);
}

QUaModbusClientWidget::~QUaModbusClientWidget()
{
    delete ui;
}

void QUaModbusClientWidget::bindClient(QUaModbusClient * client)
{
	// disable old connections
	while (m_connections.count() > 0)
	{
		QObject::disconnect(m_connections.takeFirst());
	}
	// bind edit widget
	this->bindClientWidgetEdit(client);
	// bind status widget
	this->bindClientWidgetStatus(client);
	// bind buttons
	ui->pushButtonConnect->setText(
		client->state()->value().value<QModbusState>() == QModbusState::UnconnectedState ? 
		tr("Connect") : tr("Disconnect")
	);
	m_connections <<
	QObject::connect(ui->pushButtonConnect, &QPushButton::clicked, client,
	[client]() {
		Q_CHECK_PTR(client);
		auto state = client->state();
		auto modState = state->value().value<QModbusState>();
		if (modState == QModbusState::UnconnectedState)
		{
			client->connectDevice();
			return;
		}
		client->disconnectDevice();
	});
	m_connections <<
	QObject::connect(client, &QUaModbusClient::stateChanged, ui->pushButtonConnect,
	[this](QModbusState state) {
		if (state == QModbusState::UnconnectedState)
		{
			ui->pushButtonConnect->setText(tr("Connect"));
			return;
		}
		ui->pushButtonConnect->setText(tr("Disconnect"));
	});
	m_connections <<
	QObject::connect(ui->pushButtonAddBlock, &QPushButton::clicked, client,
	[this, client]() {
		Q_CHECK_PTR(client);
		// use block edit widget
		QUaModbusDataBlockWidgetEdit * widgetNewBlock = new QUaModbusDataBlockWidgetEdit;
		QUaModbusClientDialog dialog(this);
		dialog.setWindowTitle(tr("New Modbus Block"));
		// NOTE : dialog takes ownershit
		dialog.setWidget(widgetNewBlock);
		// NOTE : call in own method to we can recall it if fails
		this->showNewBlockDialog(client, dialog);
	});
	m_connections <<
	QObject::connect(ui->pushButtonDelete, &QPushButton::clicked, client,
	[this, client]() {
		Q_CHECK_PTR(client);
		// are you sure?
		auto res = QMessageBox::question(
			this,
			tr("Delete Client Confirmation"),
			tr("Deleting client %1 will also delete all its Blocks and Values.\nWould you like to delete client %1?").arg(client->browseName()),
			QMessageBox::StandardButton::Ok,
			QMessageBox::StandardButton::Cancel
		);
		if (res != QMessageBox::StandardButton::Ok)
		{
			return;
		}
		// delete
		client->remove();
		// NOTE : removed from tree on &QObject::destroyed callback
	});
	// NOTE : apply button bound in bindClientWidgetEdit
}

void QUaModbusClientWidget::bindClientWidgetEdit(QUaModbusClient * client)
{
	// enable edit widget
	ui->widgetClientEdit->setEnabled(true);
	// show apply button
	ui->pushButtonApply->setEnabled(true);
	ui->pushButtonApply->setVisible(true);
	// bind common
	m_connections <<
	QObject::connect(client, &QObject::destroyed, ui->widgetClientEdit,
	[this]() {
		ui->widgetClientEdit->setEnabled(false);
	});
	// id
	ui->widgetClientEdit->setIdEditable(false);
	ui->widgetClientEdit->setId(client->browseName());
	// type
	ui->widgetClientEdit->setTypeEditable(false);
	ui->widgetClientEdit->setType(client->getType());
	// modbus addess
	ui->widgetClientEdit->setDeviceAddress(client->getServerAddress());
	m_connections <<
	QObject::connect(client, &QUaModbusClient::serverAddressChanged, ui->widgetClientEdit,
	[this](const quint8 &serverAddress) {
		ui->widgetClientEdit->setDeviceAddress(serverAddress);
	});
	// keep connecting
	ui->widgetClientEdit->setKeepConnecting(client->getKeepConnecting());
	m_connections <<
	QObject::connect(client, &QUaModbusClient::keepConnectingChanged, ui->widgetClientEdit,
	[this](const bool &keepConnecting) {
		ui->widgetClientEdit->setKeepConnecting(keepConnecting);
	});
	// check if tcp or serial
	switch (client->getType())
	{
	case QModbusClientType::Tcp:
		{
			auto cliTcp = dynamic_cast<QUaModbusTcpClient*>(client);
			Q_CHECK_PTR(cliTcp);
			// ip address
			ui->widgetClientEdit->setIpAddress(cliTcp->getNetworkAddress());
			m_connections <<
			QObject::connect(cliTcp, &QUaModbusTcpClient::networkAddressChanged, ui->widgetClientEdit,
			[this](const QString &strNetworkAddress) {
				ui->widgetClientEdit->setIpAddress(strNetworkAddress);
			});
			// ip port
			ui->widgetClientEdit->setNetworkPort(cliTcp->getNetworkPort());
			m_connections <<
			QObject::connect(cliTcp, &QUaModbusTcpClient::networkPortChanged, ui->widgetClientEdit,
			[this](const quint16 &networkPort) {
				ui->widgetClientEdit->setNetworkPort(networkPort);
			});
			// on apply
			m_connections <<
			QObject::connect(ui->pushButtonApply, &QPushButton::clicked, ui->widgetClientEdit,
				[cliTcp, this]() {
				// common
				cliTcp->setServerAddress(ui->widgetClientEdit->deviceAddress());
				cliTcp->setKeepConnecting(ui->widgetClientEdit->keepConnecting());
				// tcp
				cliTcp->setNetworkAddress(ui->widgetClientEdit->ipAddress());
				cliTcp->setNetworkPort(ui->widgetClientEdit->networkPort());
			});
	}
		break;
	case QModbusClientType::Serial:
	{
		auto cliSerial = dynamic_cast<QUaModbusRtuSerialClient*>(client);
		Q_CHECK_PTR(cliSerial);
		// com port
		ui->widgetClientEdit->setComPortKey(cliSerial->getComPortKey());
		m_connections <<
		QObject::connect(cliSerial, &QUaModbusRtuSerialClient::comPortChanged, ui->widgetClientEdit,
		[this](const QString &strComPort) {
			ui->widgetClientEdit->setComPort(strComPort);
		});
		// parity
		ui->widgetClientEdit->setParity(cliSerial->getParity());
		m_connections <<
		QObject::connect(cliSerial, &QUaModbusRtuSerialClient::parityChanged, ui->widgetClientEdit,
		[this](const QParity &parity) {
			ui->widgetClientEdit->setParity(parity);
		});
		// baud rate
		ui->widgetClientEdit->setBaudRate(cliSerial->getBaudRate());
		m_connections <<
		QObject::connect(cliSerial, &QUaModbusRtuSerialClient::baudRateChanged, ui->widgetClientEdit,
		[this](const QBaudRate &baudRate) {
			ui->widgetClientEdit->setBaudRate(baudRate);
		});
		// data bits
		ui->widgetClientEdit->setDataBits(cliSerial->getDataBits());
		m_connections <<
		QObject::connect(cliSerial, &QUaModbusRtuSerialClient::dataBitsChanged, ui->widgetClientEdit,
		[this](const QDataBits &dataBits) {
			ui->widgetClientEdit->setDataBits(dataBits);
		});
		// stop bits
		ui->widgetClientEdit->setStopBits(cliSerial->getStopBits());
		m_connections <<
		QObject::connect(cliSerial, &QUaModbusRtuSerialClient::stopBitsChanged, ui->widgetClientEdit,
		[this](const QStopBits &stopBits) {
			ui->widgetClientEdit->setStopBits(stopBits);
		});
		// on apply
		m_connections <<
		QObject::connect(ui->pushButtonApply, &QPushButton::clicked, ui->widgetClientEdit,
		[cliSerial, this]() {
			// common
			cliSerial->setServerAddress(ui->widgetClientEdit->deviceAddress());
			cliSerial->setKeepConnecting(ui->widgetClientEdit->keepConnecting());
			// serial
			cliSerial->setComPortKey(ui->widgetClientEdit->comPortKey());
			cliSerial->setParity(ui->widgetClientEdit->parity());
			cliSerial->setBaudRate(ui->widgetClientEdit->baudRate());
			cliSerial->setDataBits(ui->widgetClientEdit->dataBits());
			cliSerial->setStopBits(ui->widgetClientEdit->stopBits());
			});
		}
		break;
	default:
		Q_ASSERT(false);
		break;
	}
}

void QUaModbusClientWidget::bindClientWidgetStatus(QUaModbusClient * client)
{
	// enable status widget
	ui->widgetClientStatus->setEnabled(true);
	// bind
	m_connections <<
	QObject::connect(client, &QObject::destroyed, ui->widgetClientStatus,
	[this]() {
		ui->widgetClientStatus->setEnabled(false);
	});
	// status
	ui->widgetClientStatus->setStatus(client->getLastError());
	m_connections <<
	QObject::connect(client, &QUaModbusClient::lastErrorChanged, ui->widgetClientStatus,
	[this](const QModbusError & error) {
		ui->widgetClientStatus->setStatus(error);
	});
	// status
	ui->widgetClientStatus->setState(client->getState());
	m_connections <<
	QObject::connect(client, &QUaModbusClient::stateChanged, ui->widgetClientStatus,
	[this](const QModbusState & state) {
		ui->widgetClientStatus->setState(state);
	});
}

void QUaModbusClientWidget::showNewBlockDialog(QUaModbusClient * client, QUaModbusClientDialog &dialog)
{
	Q_CHECK_PTR(client);
	int res = dialog.exec();
	if (res != QDialog::Accepted)
	{
		return;
	}
	// get new client type
	auto widgetNewBlock = qobject_cast<QUaModbusDataBlockWidgetEdit*>(dialog.widget());
	Q_CHECK_PTR(widgetNewBlock);
	// get data from widget
	auto strBlockId = widgetNewBlock->id();
	// check
	auto listBlocks = client->dataBlocks();
	QString strError = listBlocks->addDataBlock(strBlockId);
	if (strError.contains("Error", Qt::CaseInsensitive))
	{
		QMessageBox::critical(this, tr("New Block Error"), strError, QMessageBox::StandardButton::Ok);
		this->showNewBlockDialog(client, dialog);
		return;
	}
	// set properties
	auto block = listBlocks->browseChild<QUaModbusDataBlock>(strBlockId);
	Q_CHECK_PTR(block);
	block->setType(widgetNewBlock->type());
	block->setAddress(widgetNewBlock->address());
	block->setSize(widgetNewBlock->size());
	block->setSamplingTime(widgetNewBlock->samplingTime());
	// NOTE : new block is added to tree using OPC UA events 
}