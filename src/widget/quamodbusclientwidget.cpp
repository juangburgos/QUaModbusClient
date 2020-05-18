#include "quamodbusclientwidget.h"
#include "ui_quamodbusclientwidget.h"

#include <QMessageBox>

#include <QUaModbusTcpClient>
#include <QUaModbusRtuSerialClient>

#include <QUaModbusClientDialog>
#include <QUaModbusDataBlockWidgetEdit>

#ifdef QUA_ACCESS_CONTROL
#include <QUaUser>
#include <QUaPermissions>
#include <QUaDockWidgetPerms>
#endif // QUA_ACCESS_CONTROL

QUaModbusClientWidget::QUaModbusClientWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QUaModbusClientWidget)
{
    ui->setupUi(this);
#ifndef QUA_ACCESS_CONTROL
	ui->pushButtonPerms->setVisible(false);
#else
	m_loggedUser = nullptr;
	m_proxyPerms = nullptr;
	ui->pushButtonPerms->setToolTip(tr(
		"Read permissions control if this client is shown.\n"
		"Write permissions control if the client parameters can be changed. "
		"Also controls if blocks can be added or removed"
	));
#endif // !QUA_ACCESS_CONTROL
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
	// check if valid
	if (!client)
	{
		this->setEnabled(false);
		return;
	}
	// bind common
	m_connections <<
	QObject::connect(client, &QObject::destroyed, this,
	[this]() {
		this->bindClient(nullptr);
	});
	// enable
	this->setEnabled(true);
	// bind edit widget
	this->bindClientWidgetEdit(client);
	// bind status widget
	this->bindClientWidgetStatus(client);
#ifdef QUA_ACCESS_CONTROL
	m_connections <<
	QObject::connect(ui->pushButtonPerms, &QPushButton::clicked, client,
	[this, client]() {
		// NOTE : call QUaModbusClientWidget::setupPermissionsModel first to set m_proxyPerms
		Q_CHECK_PTR(m_proxyPerms);
		// create permissions widget
		auto permsWidget = new QUaDockWidgetPerms;
		// configure perms widget combo
		permsWidget->setComboModel(m_proxyPerms);
		permsWidget->setPermissions(client->permissionsObject());
		// dialog
		QUaModbusClientDialog dialog(this);
		dialog.setWindowTitle(tr("Modbus Client Permissions"));
		dialog.setWidget(permsWidget);
		// exec dialog
		int res = dialog.exec();
		if (res != QDialog::Accepted)
		{
			return;
		}
		// read permissions and set them for layout list
		auto perms = permsWidget->permissions();
		perms ? client->setPermissionsObject(perms) : client->clearPermissions();
		// update widgets
		this->on_loggedUserChanged(m_loggedUser);
	});
	m_connections <<
	QObject::connect(this, &QUaModbusClientWidget::loggedUserChanged, client,
	[this, client]() {
		// client perms
		auto perms    = client ? client->permissionsObject() : nullptr;
		auto canRead  = !m_loggedUser ? false : !perms ? true : perms->canUserRead(m_loggedUser);
		auto canWrite = !m_loggedUser ? false : !perms ? true : perms->canUserWrite(m_loggedUser);
		if (!canRead)
		{
			this->setEnabled(false);
		}
		QString strToolTip = canWrite ?
			tr("") :
			tr("Do not have permissions.");
		// input widgets
		ui->widgetClientEdit->setDeviceAddressEditable (canWrite);
		ui->widgetClientEdit->setKeepConnectingEditable(canWrite);
		ui->widgetClientEdit->setIpAddressEditable     (canWrite);
		ui->widgetClientEdit->setNetworkPortEditable   (canWrite);
		ui->widgetClientEdit->setComPortEditable       (canWrite);
		ui->widgetClientEdit->setParityEditable        (canWrite);
		ui->widgetClientEdit->setBaudRateEditable      (canWrite);
		ui->widgetClientEdit->setDataBitsEditable      (canWrite);
		ui->widgetClientEdit->setStopBitsEditable      (canWrite);
		// action buttons
		ui->pushButtonApply   ->setEnabled(canWrite);
		ui->pushButtonDelete  ->setEnabled(canWrite);
		ui->pushButtonAddBlock->setEnabled(canWrite);
		ui->pushButtonClear   ->setEnabled(canWrite);
		ui->pushButtonPerms   ->setVisible(canWrite); // NOTE : only hide this one
		ui->pushButtonConnect ->setEnabled(canWrite); // NOTE : dont know if permission to connect/disconnect belongs here
		// tooltips
		ui->pushButtonApply   ->setToolTip(strToolTip);
		ui->pushButtonDelete  ->setToolTip(strToolTip);
		ui->pushButtonAddBlock->setToolTip(strToolTip);
		ui->pushButtonClear   ->setToolTip(strToolTip);
		ui->pushButtonConnect ->setToolTip(strToolTip);
	});
#endif // QUA_ACCESS_CONTROL
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
			tr("Deleting client %1 will also delete all its Blocks and Values.\nWould you like to delete client %1?").arg(client->browseName().name()),
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
	m_connections <<
	QObject::connect(ui->pushButtonClear, &QPushButton::clicked, client,
	[this, client]() {
		Q_CHECK_PTR(client);
		// are you sure?
		auto res = QMessageBox::question(
			this,
			tr("Delete All Blocks Confirmation"),
			tr("Are you sure you want to delete all blocks for client %1?\nAll their values will also be deleted.").arg(client->browseName().name()),
			QMessageBox::StandardButton::Ok,
			QMessageBox::StandardButton::Cancel
		);
		if (res != QMessageBox::StandardButton::Ok)
		{
			return;
		}
		// to clear other widgets that might be attached to objects of this tree
		emit this->aboutToClear();
		// clear
		client->dataBlocks()->clear();
	});
	// NOTE : apply button bound in bindClientWidgetEdit
}

void QUaModbusClientWidget::clear()
{
	// disable old connections
	while (m_connections.count() > 0)
	{
		QObject::disconnect(m_connections.takeFirst());
	}
	// clear edit widget
	ui->widgetClientEdit->setId("");
	ui->widgetClientEdit->setIpAddress("");
	// clear status widget
}

#ifdef QUA_ACCESS_CONTROL
void QUaModbusClientWidget::setupPermissionsModel(QSortFilterProxyModel * proxyPerms)
{
	m_proxyPerms = proxyPerms;
	Q_CHECK_PTR(m_proxyPerms);
}

void QUaModbusClientWidget::on_loggedUserChanged(QUaUser * user)
{
	m_loggedUser = user;
	emit this->loggedUserChanged();
}
#endif // QUA_ACCESS_CONTROL

void QUaModbusClientWidget::bindClientWidgetEdit(QUaModbusClient * client)
{
	// show apply button
	ui->pushButtonApply->setEnabled(true);
	ui->pushButtonApply->setVisible(true);
	// id
	ui->widgetClientEdit->setIdEditable(false);
	ui->widgetClientEdit->setId(client->browseName().name());
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
			auto cliTcp = qobject_cast<QUaModbusTcpClient*>(client);
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
			// on client connect/disconnect
			bool isConnected = cliTcp->getState() == QModbusState::ConnectedState;
			ui->widgetClientEdit->setDeviceAddressEditable(!isConnected);
			ui->widgetClientEdit->setIpAddressEditable    (!isConnected);
			ui->widgetClientEdit->setNetworkPortEditable  (!isConnected);
			m_connections <<
			QObject::connect(cliTcp, &QUaModbusTcpClient::stateChanged, ui->widgetClientEdit,
			[this](const QModbusState &state) {
				bool isConnected = state == QModbusState::ConnectedState;
				ui->widgetClientEdit->setDeviceAddressEditable(!isConnected);
				ui->widgetClientEdit->setIpAddressEditable    (!isConnected);
				ui->widgetClientEdit->setNetworkPortEditable  (!isConnected);
			});
		}
		break;
	case QModbusClientType::Serial:
		{
			auto cliSerial = qobject_cast<QUaModbusRtuSerialClient*>(client);
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
				cliSerial->setServerAddress (ui->widgetClientEdit->deviceAddress());
				cliSerial->setKeepConnecting(ui->widgetClientEdit->keepConnecting());
				// serial
				cliSerial->setComPortKey(ui->widgetClientEdit->comPortKey());
				cliSerial->setParity    (ui->widgetClientEdit->parity()    );
				cliSerial->setBaudRate  (ui->widgetClientEdit->baudRate()  );
				cliSerial->setDataBits  (ui->widgetClientEdit->dataBits()  );
				cliSerial->setStopBits  (ui->widgetClientEdit->stopBits()  );
			});
			// on client connect/disconnect
			bool isConnected = cliSerial->getState() == QModbusState::ConnectedState;
			ui->widgetClientEdit->setDeviceAddressEditable(!isConnected);
			ui->widgetClientEdit->setComPortEditable      (!isConnected);
			ui->widgetClientEdit->setParityEditable       (!isConnected);
			ui->widgetClientEdit->setBaudRateEditable     (!isConnected);
			ui->widgetClientEdit->setDataBitsEditable     (!isConnected);
			ui->widgetClientEdit->setStopBitsEditable     (!isConnected);
			m_connections <<
			QObject::connect(cliSerial, &QUaModbusRtuSerialClient::stateChanged, ui->widgetClientEdit,
			[this](const QModbusState &state) {
				bool isConnected = state == QModbusState::ConnectedState;
				ui->widgetClientEdit->setDeviceAddressEditable(!isConnected);
				ui->widgetClientEdit->setComPortEditable      (!isConnected);
				ui->widgetClientEdit->setParityEditable       (!isConnected);
				ui->widgetClientEdit->setBaudRateEditable     (!isConnected);
				ui->widgetClientEdit->setDataBitsEditable     (!isConnected);
				ui->widgetClientEdit->setStopBitsEditable     (!isConnected);
				
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
	// status
	ui->widgetClientStatus->setStatus(client->getLastError());
	m_connections <<
	QObject::connect(client, &QUaModbusClient::lastErrorChanged, ui->widgetClientStatus,
	[this](const QModbusError & error) {
		ui->widgetClientStatus->setStatus(error);
	});
	// state
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