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
			cliTcp->setServerAddress (widgetNewClient->deviceAddress());
			cliTcp->setKeepConnecting(widgetNewClient->keepConnecting());
			// tcp
			cliTcp->setNetworkAddress(widgetNewClient->ipAddress());
			cliTcp->setNetworkPort   (widgetNewClient->networkPort());
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
			cliSerial->setServerAddress (widgetNewClient->deviceAddress());
			cliSerial->setKeepConnecting(widgetNewClient->keepConnecting());
			// serial
			cliSerial->setComPortKey(widgetNewClient->comPortKey());
			cliSerial->setParity    (widgetNewClient->parity()    );
			cliSerial->setBaudRate  (widgetNewClient->baudRate()  );
			cliSerial->setDataBits  (widgetNewClient->dataBits()  );
			cliSerial->setStopBits  (widgetNewClient->stopBits()  );
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
	auto client  = m_listClients->browseChild<QUaModbusClient>(strClientId);
	Q_ASSERT_X(client, "QUaModbusClientWidget", "Client instance must already exist in OPC UA");
	auto parent = m_modelClients.invisibleRootItem();

	// object column
	auto row  = parent->rowCount();
	auto iObj = new QStandardItem(strClientId);
	parent->setChild(row, (int)Headers::Objects, iObj);

	// status column
	auto enumState = QMetaEnum::fromType<QModbusState>();
	auto state     = client->state();
	auto modState  = state->value().value<QModbusState>();
	auto strState  = QString(enumState.valueToKey(modState));
	auto iStat     = new QStandardItem(strState);
	parent->setChild(row, (int)Headers::Status, iStat);
	QObject::connect(client, &QUaModbusClient::stateChanged, this,
	[iStat, enumState](QModbusState state) {
		Q_CHECK_PTR(iStat);
		auto strState = QString(enumState.valueToKey(state));
		iStat->setText(strState);
	});

	// options
	auto iActs = new QStandardItem();
	parent->setChild(row, (int)Headers::Actions, iActs);
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
	QObject::connect(pButCon, &QPushButton::clicked, [this, client, pButCon]() {
		Q_CHECK_PTR(client);
		auto state    = client->state();
		auto modState = state->value().value<QModbusState>();
		if (modState == QModbusState::UnconnectedState)
		{
			client->connectDevice();
			return;
		}
		client->disconnectDevice();
	});
	QObject::connect(client, &QUaModbusClient::stateChanged, this,
	[pButCon](QModbusState state) {
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
	QObject::connect(pButBlk, &QPushButton::clicked, [this, client]() {
		Q_CHECK_PTR(client);
		// use block edit widget
		QUaModbusDataBlockWidgetEdit * widgetNewBlock = new QUaModbusDataBlockWidgetEdit;
		QUaModbusClientDialog dialog;
		// NOTE : dialog takes ownershit
		dialog.setWidget(widgetNewBlock);
		// NOTE : call in own method to we can recall it if fails
		this->showNewBlockDialog(client, dialog);
	});
	// delete button
	pButDel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	pButDel->setText(tr("Delete"));
	pButDel->setObjectName("Delete");
	pButDel->setFocusPolicy(Qt::FocusPolicy::NoFocus);
	//pButDel->setEnabled(this->allowActions()); // TODO
	//!this->allowActions() ? pButDel->setVisible(false) : nullptr; // NOTE : fixes flicker
	QObject::connect(pButDel, &QPushButton::clicked, [this, client, iObj]() {
		Q_CHECK_PTR(client);
		Q_CHECK_PTR(iObj);
		client->remove();
		m_modelClients.removeRows(iObj->index().row(), 1);
	});
	// layout
	pLayout->addWidget(pButCon);
	pLayout->addWidget(pButBlk);
	pLayout->addWidget(pButDel);
	pLayout->setContentsMargins(5, 0, 5, 0);
	pWidget->setLayout(pLayout);
	ui->treeViewModbus->setIndexWidget(m_proxyClients.mapFromSource(iActs->index()), pWidget);

	// subscribe to block addition
	// NOTE : needs to be a queued connection because we want to wait until browseName is set
	auto listBlocks = client->dataBlocks();
	QObject::connect(listBlocks, &QUaNode::childAdded, this,
	[this, client, iObj](QUaNode * node) {
		auto block = dynamic_cast<QUaModbusDataBlock*>(node);
		Q_CHECK_PTR(block);
		// add to gui
		QString strBlockId = block->browseName();
		Q_ASSERT(!strBlockId.isEmpty() && !strBlockId.isNull());
		this->handleBlockAdded(client, iObj, strBlockId);
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
	// set properties
	auto block = listBlocks->browseChild<QUaModbusDataBlock>(strBlockId);
	Q_CHECK_PTR(block);
	block->setType        (widgetNewBlock->type()        );
	block->setAddress     (widgetNewBlock->address()     );
	block->setSize        (widgetNewBlock->size()        );
	block->setSamplingTime(widgetNewBlock->samplingTime());
	// NOTE : new block is added to GUI using OPC UA events 
}

void QUaModbusClientWidget::handleBlockAdded(QUaModbusClient * client, QStandardItem * parent, const QString & strBlockId)
{
	// get block
	auto listBlocks = client->dataBlocks();
	auto block      = listBlocks->browseChild<QUaModbusDataBlock>(strBlockId);
	Q_ASSERT_X(block, "QUaModbusClientWidget", "Block instance must already exist in OPC UA");

	// object column
	auto row = parent->rowCount();
	auto iObj = new QStandardItem(strBlockId);
	parent->setChild(row, (int)Headers::Objects, iObj);

	// status column
	auto enumError = QMetaEnum::fromType<QModbusError>();
	auto error     = block->lastError();
	auto modError  = error->value().value<QModbusError>();
	auto strError  = QString(enumError.valueToKey(modError));
	auto iErr      = new QStandardItem(strError);
	parent->setChild(row, (int)Headers::Status, iErr);
	QObject::connect(block, &QUaModbusDataBlock::lastErrorChanged, this,
	[iErr, enumError](QModbusError error) {
		Q_CHECK_PTR(iErr);
		auto strState = QString(enumError.valueToKey(error));
		iErr->setText(strState);
	});

	// options
	auto iActs = new QStandardItem();
	parent->setChild(row, (int)Headers::Actions, iActs);
	QWidget     *pWidget = new QWidget;
	QHBoxLayout *pLayout = new QHBoxLayout;
	QPushButton *pButVal = new QPushButton;
	QPushButton *pButDel = new QPushButton;
	// add value button
	pButVal->setText(tr("Add Value"));
	pButVal->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	pButVal->setObjectName("AddValue");
	pButVal->setFocusPolicy(Qt::FocusPolicy::NoFocus);
	//pButDel->setEnabled(this->allowActions()); // TODO
	//!this->allowActions() ? pButDel->setVisible(false) : nullptr; // NOTE : fixes flicker
	QObject::connect(pButVal, &QPushButton::clicked, [this, block]() {
		Q_CHECK_PTR(block);
		//// use value edit widget
		//QUaModbusValueWidgetEdit * widgetNewValue = new QUaModbusValueWidgetEdit;
		//QUaModbusClientDialog dialog;
		//// NOTE : dialog takes ownershit
		//dialog.setWidget(widgetNewValue);
		//// NOTE : call in own method to we can recall it if fails
		//this->showNewValueDialog(client, dialog);
	});
	// delete button
	pButDel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	pButDel->setText(tr("Delete"));
	pButDel->setObjectName("Delete");
	pButDel->setFocusPolicy(Qt::FocusPolicy::NoFocus);
	//pButDel->setEnabled(this->allowActions()); // TODO
	//!this->allowActions() ? pButDel->setVisible(false) : nullptr; // NOTE : fixes flicker
	QObject::connect(pButDel, &QPushButton::clicked, [block, parent, iObj]() {
		Q_CHECK_PTR(block);
		Q_CHECK_PTR(iObj);
		block->remove();
		parent->removeRows(iObj->index().row(), 1);
	});
	// layout
	pLayout->addWidget(pButVal);
	pLayout->addWidget(pButDel);
	pLayout->setContentsMargins(5, 0, 5, 0);
	pWidget->setLayout(pLayout);
	ui->treeViewModbus->setIndexWidget(m_proxyClients.mapFromSource(iActs->index()), pWidget);

}
