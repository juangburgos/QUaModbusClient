#include "quamodbusclientwidget.h"
#include "ui_quamodbusclientwidget.h"

#include <QMessageBox>

#include <QUaModbusClient>
#include <QUaModbusClientList>
#include <QUaModbusClientDialog>
#include <QUaModbusClientWidgetEdit>
#include <QUaModbusTcpClient>
#include <QUaModbusRtuSerialClient>
#include <QUaModbusDataBlock>
#include <QUaModbusDataBlockWidgetEdit>

typedef QModbusDevice::State QModbusState;

QUaModbusClientWidget::QUaModbusClientWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QUaModbusClientWidget)
{
    ui->setupUi(this);
	m_listClients = nullptr;
	// setup params tree model
	m_modelClients.setColumnCount((int)Headers::Invalid);
	QStringList paramHeaders;
	for (int i = (int)Headers::Objects; i < (int)Headers::Invalid; i++)
	{
		paramHeaders << QString(QMetaEnum::fromType<Headers>().valueToKey(i));
	}
	m_modelClients.setHorizontalHeaderLabels(paramHeaders);
	// setup params sort filter
	m_proxyClients.setSourceModel(&m_modelClients);
	// setup params table
	ui->treeViewModbus->setModel(&m_proxyClients);
	ui->treeViewModbus->setAlternatingRowColors(true);
	//ui->treeViewModbus->horizontalHeader()->setStretchLastSection(true);
	ui->treeViewModbus->setSortingEnabled(true);
	ui->treeViewModbus->setSelectionBehavior(QAbstractItemView::SelectRows);
}

QUaModbusClientWidget::~QUaModbusClientWidget()
{
    delete ui;
}

QUaModbusClientList * QUaModbusClientWidget::clientList() const
{
	return m_listClients;
}

void QUaModbusClientWidget::setClientList(QUaModbusClientList * listClients)
{
	// check valid arg
	Q_ASSERT(listClients);
	if (!listClients) { return; }
	// check not set before
	Q_ASSERT(!m_listClients);
	if (m_listClients) { return; }
	// set
	m_listClients = listClients;

	// TODO : load initial

	// subscribe to client added
	// NOTE : needs to be a queued connection because we want to wait until browseName is set
	QObject::connect(listClients, &QUaNode::childAdded, this,
	[this](QUaNode * node) {
		auto client = dynamic_cast<QUaModbusClient*>(node);
		Q_CHECK_PTR(client);
		// add to gui
		QString strClientId = client->browseName();
		Q_ASSERT(!strClientId.isEmpty() && !strClientId.isNull());
		this->handleClientAdded(strClientId);
	}, Qt::QueuedConnection);
}

void QUaModbusClientWidget::on_pushButtonAddClient_clicked()
{
	QUaModbusClientWidgetEdit * widgetNewClient = new QUaModbusClientWidgetEdit;
	QUaModbusClientDialog dialog;
	// NOTE : dialog takes ownershit
	dialog.setWidget(widgetNewClient);
	// NOTE : call in own method to we can recall it if fails
	this->showNewClientDialog(dialog);
}

void QUaModbusClientWidget::showNewClientDialog(QUaModbusClientDialog & dialog)
{
	int res = dialog.exec();
	if (res != QDialog::Accepted)
	{
		return;
	}
	// get new client type
	auto widgetNewClient = qobject_cast<QUaModbusClientWidgetEdit*>(dialog.widget());
	Q_CHECK_PTR(widgetNewClient);
	// get data from widget
	auto cliType = widgetNewClient->type();
	auto strId   = widgetNewClient->id();
	switch (cliType)
	{
	case QUaModbusClientWidgetEdit::Tcp:
		{
			// add to OPC UA
			QString strError = m_listClients->addTcpClient(strId);
			if (strError.contains("Error", Qt::CaseInsensitive))
			{
				QMessageBox::critical(this, tr("New TCP Client Error"), strError, QMessageBox::StandardButton::Ok);
				this->showNewClientDialog(dialog);
				return;
			}
			// set properties
			auto cliTcp = m_listClients->browseChild<QUaModbusTcpClient>(strId);
			Q_CHECK_PTR(cliTcp);
			// common
			cliTcp->serverAddress ()->setValue(widgetNewClient->deviceAddress());
			cliTcp->keepConnecting()->setValue(widgetNewClient->keepConnecting());
			// tcp
			cliTcp->networkAddress()->setValue(widgetNewClient->ipAddress());
			cliTcp->networkPort   ()->setValue(widgetNewClient->networkPort());
		}
		break;
	case QUaModbusClientWidgetEdit::Serial:
		{
			// add to OPC UA
			QString strError = m_listClients->addRtuSerialClient(strId);
			if (strError.contains("Error", Qt::CaseInsensitive))
			{
				QMessageBox::critical(this, tr("New Serial Client Error"), strError, QMessageBox::StandardButton::Ok);
				this->showNewClientDialog(dialog);
				return;
			}
			// set properties
			auto cliSerial = m_listClients->browseChild<QUaModbusRtuSerialClient>(strId);
			Q_CHECK_PTR(cliSerial);
			// common
			cliSerial->serverAddress ()->setValue(widgetNewClient->deviceAddress());
			cliSerial->keepConnecting()->setValue(widgetNewClient->keepConnecting());
			// serial
			cliSerial->comPort ()->setValue(widgetNewClient->comPortKey());
			cliSerial->parity  ()->setValue(widgetNewClient->parity()    );
			cliSerial->baudRate()->setValue(widgetNewClient->baudRate()  );
			cliSerial->dataBits()->setValue(widgetNewClient->dataBits()  );
			cliSerial->stopBits()->setValue(widgetNewClient->stopBits()  );
		}
		break;
	case QUaModbusClientWidgetEdit::Invalid:
	default:
		Q_ASSERT(false);
		break;
	}
	// NOTE : new client is added to GUI using OPC UA events 
}

// TODO : do something with lastError, maybe somewhere else?

void QUaModbusClientWidget::handleClientAdded(const QString & strClientId)
{
	// get client
	auto cli  = m_listClients->browseChild<QUaModbusClient>(strClientId);
	Q_ASSERT_X(cli, "QUaModbusClientWidget", "Client instance must already exist in OPC UA");
	auto root = m_modelClients.invisibleRootItem();
	// object column
	auto row  = root->rowCount();
	auto iobj = new QStandardItem(strClientId);
	root->setChild(row, (int)Headers::Objects, iobj);
	// status column
	auto enumState = QMetaEnum::fromType<QModbusState>();
	auto state     = cli->state();
	auto modState  = state->value().value<QModbusState>();
	auto strState  = QString(enumState.valueToKey(modState));
	auto istat     = new QStandardItem(strState);
	root->setChild(row, (int)Headers::Status, istat);
	QObject::connect(cli, &QUaModbusClient::stateChanged, this,
	[istat, state, enumState](QModbusDevice::State state) {
		Q_CHECK_PTR(istat);
		auto strState = QString(enumState.valueToKey(state));
		istat->setText(strState);
	});
	// options
	auto iacts = new QStandardItem();
	root->setChild(row, (int)Headers::Actions, iacts);
	QWidget     *pWidget = new QWidget;
	QHBoxLayout *pLayout = new QHBoxLayout;
	QPushButton *pButCon = new QPushButton;
	QPushButton *pButBlk = new QPushButton;
	QPushButton *pButDel = new QPushButton;
	// connect button
	pButCon->setText(tr("Connect"));
	pButCon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	pButCon->setObjectName("Connect");
	pButCon->setFocusPolicy(Qt::FocusPolicy::NoFocus);
	//pButDel->setEnabled(this->allowActions()); // TODO
	//!this->allowActions() ? pButDel->setVisible(false) : nullptr; // NOTE : fixes flicker
	QObject::connect(pButCon, &QPushButton::clicked, [this, cli, pButCon]() {
		Q_CHECK_PTR(cli);
		auto state    = cli->state();
		auto modState = state->value().value<QModbusState>();
		if (modState == QModbusState::UnconnectedState)
		{
			cli->connectDevice();
			return;
		}
		cli->disconnectDevice();
	});
	QObject::connect(cli, &QUaModbusClient::stateChanged, this,
	[pButCon](QModbusDevice::State state) {
		if (state == QModbusState::UnconnectedState)
		{
			pButCon->setText(tr("Connect"));
			return;
		}
		pButCon->setText(tr("Disconnect"));
	});

	// add block button
	pButBlk->setText(tr("Add Block"));
	pButBlk->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	pButBlk->setObjectName("AddBlock");
	pButBlk->setFocusPolicy(Qt::FocusPolicy::NoFocus);
	//pButDel->setEnabled(this->allowActions()); // TODO
	//!this->allowActions() ? pButDel->setVisible(false) : nullptr; // NOTE : fixes flicker
	QObject::connect(pButBlk, &QPushButton::clicked, [this, cli]() {
		Q_CHECK_PTR(cli);
		// use block edit widget
		QUaModbusDataBlockWidgetEdit * widgetNewBlock = new QUaModbusDataBlockWidgetEdit;
		QUaModbusClientDialog dialog;
		// NOTE : dialog takes ownershit
		dialog.setWidget(widgetNewBlock);
		// NOTE : call in own method to we can recall it if fails
		this->showNewBlockDialog(cli, dialog);
	});

	// delete button
	pButDel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	pButDel->setText(tr("Delete"));
	pButDel->setObjectName("Delete");
	pButDel->setFocusPolicy(Qt::FocusPolicy::NoFocus);
	//pButDel->setEnabled(this->allowActions()); // TODO
	//!this->allowActions() ? pButDel->setVisible(false) : nullptr; // NOTE : fixes flicker
	QObject::connect(pButDel, &QPushButton::clicked, [this, cli, iobj]() {
		Q_CHECK_PTR(cli);
		Q_CHECK_PTR(iobj);
		cli->remove();
		m_modelClients.removeRows(iobj->index().row(), 1);
	});

	// layout
	pLayout->addWidget(pButCon);
	pLayout->addWidget(pButBlk);
	pLayout->addWidget(pButDel);
	pLayout->setContentsMargins(5, 0, 5, 0);
	pWidget->setLayout(pLayout);
	ui->treeViewModbus->setIndexWidget(m_proxyClients.mapFromSource(iacts->index()), pWidget);

	// subscribe to block addition
	// NOTE : needs to be a queued connection because we want to wait until browseName is set
	auto listBlocks = cli->dataBlocks();
	QObject::connect(listBlocks, &QUaNode::childAdded, this,
	[this, strClientId](QUaNode * node) {
		auto block = dynamic_cast<QUaModbusDataBlock*>(node);
		Q_CHECK_PTR(block);
		// add to gui
		QString strBlockId = block->browseName();
		Q_ASSERT(!strBlockId.isEmpty() && !strBlockId.isNull());
		this->handleBlockAdded(strClientId, strBlockId);
	}, Qt::QueuedConnection);

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
	// setup new block
	auto blockType  = widgetNewBlock->type();

	// TODO :

}

void QUaModbusClientWidget::handleBlockAdded(const QString & strClientId, const QString & strBlockId)
{
	// TODO :
}
