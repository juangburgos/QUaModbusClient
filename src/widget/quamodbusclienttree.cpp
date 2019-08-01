#include "quamodbusclienttree.h"
#include "ui_quamodbusclienttree.h"

#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QStandardPaths>

#include <QUaModbusClientDialog>
#include <QUaModbusClientList>
#include <QUaModbusClient>
#include <QUaModbusTcpClient>
#include <QUaModbusRtuSerialClient>
#include <QUaModbusDataBlockList>
#include <QUaModbusDataBlock>
#include <QUaModbusValueList>
#include <QUaModbusValue>

#include <QUaModbusClientWidgetEdit>

int QUaModbusClientTree::SelectTypeRole = Qt::UserRole + 1;
int QUaModbusClientTree::PointerRole = Qt::UserRole + 2;

QUaModbusClientTree::QUaModbusClientTree(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::QUaModbusClientTree)
{
	ui->setupUi(this);
	m_listClients = nullptr;
	if (QMetaType::type("QModbusSelectType") == QMetaType::UnknownType)
	{
		qRegisterMetaType<QModbusSelectType>("QModbusSelectType");
	}
	if (QMetaType::type("QUaNode*") == QMetaType::UnknownType)
	{
		qRegisterMetaType<QUaNode*>("QUaNode*");
	}
	this->setupImportButton();
	this->setupExportButton();
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
	// setup params tree
	ui->treeViewModbus->setModel(&m_proxyClients);
	ui->treeViewModbus->setAlternatingRowColors(true);
	ui->treeViewModbus->setSortingEnabled(true);
	ui->treeViewModbus->sortByColumn((int)Headers::Objects, Qt::SortOrder::AscendingOrder);
	ui->treeViewModbus->setSelectionBehavior(QAbstractItemView::SelectRows);
	// setup tree interactions
	QObject::connect(ui->treeViewModbus->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
	[this](const QModelIndex &current, const QModelIndex &previous) {
		auto itemPrev = m_modelClients.itemFromIndex(m_proxyClients.mapToSource(previous));
		auto itemCurr = m_modelClients.itemFromIndex(m_proxyClients.mapToSource(current));
		// previous
		QModbusSelectType typePrev = QModbusSelectType::Invalid;
		QUaNode * nodePrev = nullptr;
		if (itemPrev)
		{
			nodePrev = itemPrev->data(QUaModbusClientTree::PointerRole).value<QUaNode*>();
			typePrev = itemPrev->data(QUaModbusClientTree::SelectTypeRole).value<QModbusSelectType>();
			switch (typePrev)
			{
			case QModbusSelectType::QUaModbusClient:
				nodePrev = dynamic_cast<QUaModbusClient*>(nodePrev);
				break;
			case QModbusSelectType::QUaModbusDataBlock:
				nodePrev = dynamic_cast<QUaModbusDataBlock*>(nodePrev);
				break;
			case QModbusSelectType::QUaModbusValue:
				nodePrev = dynamic_cast<QUaModbusValue*>(nodePrev);
				break;
			default:
				nodePrev = nullptr;
				break;
			}
		}
		// current
		QModbusSelectType typeCurr = QModbusSelectType::Invalid;
		QUaNode * nodeCurr = nullptr;
		if (itemCurr)
		{
			nodeCurr = itemCurr->data(QUaModbusClientTree::PointerRole).value<QUaNode*>();
			typeCurr = itemCurr->data(QUaModbusClientTree::SelectTypeRole).value<QModbusSelectType>();
			switch (typeCurr)
			{
			case QModbusSelectType::QUaModbusClient:
				nodeCurr = dynamic_cast<QUaModbusClient*>(nodeCurr);
				break;
			case QModbusSelectType::QUaModbusDataBlock:
				nodeCurr = dynamic_cast<QUaModbusDataBlock*>(nodeCurr);
				break;
			case QModbusSelectType::QUaModbusValue:
				nodeCurr = dynamic_cast<QUaModbusValue*>(nodeCurr);
				break;
			default:
				nodeCurr = nullptr;
				break;
			}
		}
		// emit
		emit this->nodeSelectionChanged(nodePrev, typePrev, nodeCurr, typeCurr);
	});
}

QUaModbusClientTree::~QUaModbusClientTree()
{
	delete ui;
}

QUaModbusClientList * QUaModbusClientTree::clientList() const
{
	return m_listClients;
}

void QUaModbusClientTree::setClientList(QUaModbusClientList * listClients)
{
	// check valid arg
	Q_ASSERT(listClients);
	if (!listClients) { return; }
	// check not set before
	Q_ASSERT(!m_listClients);
	if (m_listClients) { return; }
	// set
	m_listClients = listClients;

	// subscribe to client added
	// NOTE : needs to be a queued connection because we want to wait until browseName is set
	QObject::connect(listClients, &QUaNode::childAdded, this,
	[this](QUaNode * node) {
		auto client = dynamic_cast<QUaModbusClient*>(node);
		Q_CHECK_PTR(client);
		// add to gui
		auto item = this->handleClientAdded(client);
		// select newly created
		auto index = m_proxyClients.mapFromSource(item->index());
		ui->treeViewModbus->setCurrentIndex(index);
	}, Qt::QueuedConnection);

	// add already existing clients
	for (int i = 0; i < listClients->clients().count(); i++)
	{
		auto client = listClients->clients().at(i);
		Q_CHECK_PTR(client);
		// add to gui
		this->handleClientAdded(client);
	}
}

void QUaModbusClientTree::on_pushButtonAddClient_clicked()
{
	QUaModbusClientWidgetEdit * widgetNewClient = new QUaModbusClientWidgetEdit;
	QUaModbusClientDialog dialog(this);
	dialog.setWindowTitle(tr("New Modbus Client"));
	// NOTE : dialog takes ownershit
	dialog.setWidget(widgetNewClient);
	// NOTE : call in own method to we can recall it if fails
	this->showNewClientDialog(dialog);
}

void QUaModbusClientTree::setupImportButton()
{
	ui->toolButtonImport->setPopupMode(QToolButton::MenuButtonPopup);
	// menu
	auto importMenu = new QMenu(ui->toolButtonImport);
	importMenu->addAction(tr("Clients"), this, 
	[this](){
		QString strContents = loadContentsCsvFromFile();
		if (strContents.isEmpty())
		{
			return;
		}
		QString strError = m_listClients->setCsvClients(strContents);
		this->displayCsvLoadResult(strError);
	});
	importMenu->addAction(tr("Blocks"), this, 
	[this](){
		QString strContents = loadContentsCsvFromFile();
		if (strContents.isEmpty())
		{
			return;
		}
		QString strError = m_listClients->setCsvBlocks(strContents);
		this->displayCsvLoadResult(strError);
	});
	importMenu->addAction(tr("Values"), this, 
	[this](){
		QString strContents = loadContentsCsvFromFile();
		if (strContents.isEmpty())
		{
			return;
		}
		QString strError = m_listClients->setCsvValues(strContents);
		this->displayCsvLoadResult(strError);
	});
	// set menu
	ui->toolButtonImport->setMenu(importMenu);
	// default action
	auto defaultAction = new QAction(tr("Import CSV"), ui->toolButtonImport);
	QObject::connect(defaultAction, &QAction::triggered, ui->toolButtonImport,
	[this]() {
		ui->toolButtonImport->showMenu();
	});
	ui->toolButtonImport->setDefaultAction(defaultAction);
}

void QUaModbusClientTree::setupExportButton()
{
	ui->toolButtonExport->setPopupMode(QToolButton::MenuButtonPopup);
	// menu
	auto exportMenu = new QMenu(ui->toolButtonExport);
	exportMenu->addAction(tr("Clients"), this, 
	[this](){
		this->saveContentsCsvToFile(m_listClients->csvClients());
	});
	exportMenu->addAction(tr("Blocks"), this, 
	[this](){
		this->saveContentsCsvToFile(m_listClients->csvBlocks());
	});
	exportMenu->addAction(tr("Values"), this, 
	[this](){
		this->saveContentsCsvToFile(m_listClients->csvValues());
	});
	// set menu
	ui->toolButtonExport->setMenu(exportMenu);
	// default action
	auto defaultAction = new QAction(tr("Export CSV"), ui->toolButtonExport);
	QObject::connect(defaultAction, &QAction::triggered, ui->toolButtonExport,
	[this]() {
		ui->toolButtonExport->showMenu();
	});
	ui->toolButtonExport->setDefaultAction(defaultAction);
}

void QUaModbusClientTree::showNewClientDialog(QUaModbusClientDialog & dialog)
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
	auto strId = widgetNewClient->id();
	switch (cliType)
	{
	case QModbusClientType::Tcp:
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
		cliTcp->setServerAddress(widgetNewClient->deviceAddress());
		cliTcp->setKeepConnecting(widgetNewClient->keepConnecting());
		// tcp
		cliTcp->setNetworkAddress(widgetNewClient->ipAddress());
		cliTcp->setNetworkPort(widgetNewClient->networkPort());
		// start connecting to client if keepConnecting was set to true
		if (cliTcp->getKeepConnecting())
		{
			cliTcp->connectDevice();
		}
	}
	break;
	case QModbusClientType::Serial:
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
		cliSerial->setServerAddress(widgetNewClient->deviceAddress());
		cliSerial->setKeepConnecting(widgetNewClient->keepConnecting());
		// serial
		cliSerial->setComPortKey(widgetNewClient->comPortKey());
		cliSerial->setParity(widgetNewClient->parity());
		cliSerial->setBaudRate(widgetNewClient->baudRate());
		cliSerial->setDataBits(widgetNewClient->dataBits());
		cliSerial->setStopBits(widgetNewClient->stopBits());
		// start connecting to client if keepConnecting was set to true
		if (cliSerial->getKeepConnecting())
		{
			cliSerial->connectDevice();
		}
	}
	break;
	case QModbusClientType::Invalid:
	default:
		Q_ASSERT(false);
		break;
	}
	// NOTE : new client is added to GUI using OPC UA events 
}

QStandardItem *  QUaModbusClientTree::handleClientAdded(QUaModbusClient * client)
{
	Q_ASSERT_X(client, "QUaModbusClientWidget", "Client instance must already exist in OPC UA");
	// get client id
	QString strClientId = client->browseName();
	Q_ASSERT(!strClientId.isEmpty() && !strClientId.isNull());
	auto parent = m_modelClients.invisibleRootItem();

	// object column
	auto row  = parent->rowCount();
	auto iObj = new QStandardItem(strClientId);
	parent->setChild(row, (int)Headers::Objects, iObj);
	// set data
	iObj->setData(QVariant::fromValue(QModbusSelectType::QUaModbusClient), QUaModbusClientTree::SelectTypeRole);
	iObj->setData(QVariant::fromValue(client), QUaModbusClientTree::PointerRole);

	// status column
	auto enumState = QMetaEnum::fromType<QModbusState>();
	auto state     = client->state();
	auto modState  = state->value().value<QModbusState>();
	auto strState  = QString(enumState.valueToKey(modState));
	auto enumError = QMetaEnum::fromType<QModbusError>();
	auto error     = client->lastError();
	auto modError  = error->value().value<QModbusError>();
	auto strError  = QString(enumError.valueToKey(modError));
	auto iStat     = new QStandardItem(strState + " | " + strError);
	parent->setChild(row, (int)Headers::Status, iStat);
	// set data
	iStat->setData(QVariant::fromValue(QModbusSelectType::QUaModbusClient), QUaModbusClientTree::SelectTypeRole);
	iStat->setData(QVariant::fromValue(client), QUaModbusClientTree::PointerRole);
	// changes
	QObject::connect(client, &QUaModbusClient::stateChanged, this,
	[iStat, client, enumState, enumError](QModbusState state) {
		Q_CHECK_PTR(iStat);
		auto error    = client->getLastError();
		auto strState = QString(enumState.valueToKey(state));
		auto strError = QString(enumError.valueToKey(error));
		iStat->setText(strState + " | " + strError);
	});
	QObject::connect(client, &QUaModbusClient::lastErrorChanged, this,
	[iStat, client, enumState, enumError](const QModbusError &error) {
		Q_CHECK_PTR(iStat);
		auto state    = client->getState();
		auto strState = QString(enumState.valueToKey(state));
		auto strError = QString(enumError.valueToKey(error));
		iStat->setText(strState + " | " + strError);
	});

	// ua delete
	// NOTE : set this as receiver, so callback is not called if this has been deleted
	QObject::connect(client, &QObject::destroyed, this,
	[this, iObj]() {
		Q_CHECK_PTR(iObj);
		// remove from tree
		m_modelClients.removeRows(iObj->index().row(), 1);
	});

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
		auto item = this->handleBlockAdded(client, iObj, strBlockId);
		// select newly created
		auto index = m_proxyClients.mapFromSource(item->index());
		ui->treeViewModbus->setCurrentIndex(index);
	}, Qt::QueuedConnection);

	// add already existing blocks
	for (int i = 0; i < listBlocks->blocks().count(); i++)
	{
		auto block = listBlocks->blocks().at(i);
		// add to gui
		QString strBlockId = block->browseName();
		Q_ASSERT(!strBlockId.isEmpty() && !strBlockId.isNull());
		this->handleBlockAdded(client, iObj, strBlockId);
	}

	return iObj;
}

QStandardItem *  QUaModbusClientTree::handleBlockAdded(QUaModbusClient * client, QStandardItem * parent, const QString & strBlockId)
{
	// get block
	auto listBlocks = client->dataBlocks();
	auto block      = listBlocks->browseChild<QUaModbusDataBlock>(strBlockId);
	Q_ASSERT_X(block, "QUaModbusClientWidget", "Block instance must already exist in OPC UA");

	// object column
	auto row  = parent->rowCount();
	auto iObj = new QStandardItem(strBlockId);
	parent->setChild(row, (int)Headers::Objects, iObj);
	// set data
	iObj->setData(QVariant::fromValue(QModbusSelectType::QUaModbusDataBlock), QUaModbusClientTree::SelectTypeRole);
	iObj->setData(QVariant::fromValue(block), QUaModbusClientTree::PointerRole);

	// status column
	auto enumError = QMetaEnum::fromType<QModbusError>();
	auto error     = block->lastError();
	auto modError  = error->value().value<QModbusError>();
	auto strError  = QString(enumError.valueToKey(modError));
	auto iErr      = new QStandardItem(strError);
	parent->setChild(row, (int)Headers::Status, iErr);
	// set data
	iErr->setData(QVariant::fromValue(QModbusSelectType::QUaModbusDataBlock), QUaModbusClientTree::SelectTypeRole);
	iErr->setData(QVariant::fromValue(block), QUaModbusClientTree::PointerRole);
	// changes
	QObject::connect(block, &QUaModbusDataBlock::lastErrorChanged, this,
	[iErr, enumError](const QModbusError &error) {
		Q_CHECK_PTR(iErr);
		auto strState = QString(enumError.valueToKey(error));
		iErr->setText(strState);
	});

	// ua delete
	// NOTE : set client as receiver, so callback is not called if client has been deleted
	QObject::connect(block, &QObject::destroyed, client,
	[parent, iObj]() {
		Q_CHECK_PTR(parent);
		Q_CHECK_PTR(iObj);
		parent->removeRows(iObj->index().row(), 1);
	});

	// subscribe to value addition
	// NOTE : needs to be a queued connection because we want to wait until browseName is set
	auto listValues = block->values();
	QObject::connect(listValues, &QUaNode::childAdded, this,
	[this, block, iObj](QUaNode * node) {
		auto value = dynamic_cast<QUaModbusValue*>(node);
		Q_CHECK_PTR(value);
		// add to gui
		QString strValueId = value->browseName();
		Q_ASSERT(!strValueId.isEmpty() && !strValueId.isNull());
		auto item = this->handleValueAdded(block, iObj, strValueId);
		// select newly created
		auto index = m_proxyClients.mapFromSource(item->index());
		ui->treeViewModbus->setCurrentIndex(index);
	}, Qt::QueuedConnection);

	// add already existing values
	for (int i = 0; i < listValues->values().count(); i++)
	{
		auto value = listValues->values().at(i);
		// add to gui
		QString strValueId = value->browseName();
		Q_ASSERT(!strValueId.isEmpty() && !strValueId.isNull());
		this->handleValueAdded(block, iObj, strValueId);
	}

	return iObj;
}

QStandardItem *  QUaModbusClientTree::handleValueAdded(QUaModbusDataBlock * block, QStandardItem * parent, const QString & strValueId)
{
	// get block
	auto listValues = block->values();
	auto value      = listValues->browseChild<QUaModbusValue>(strValueId);
	Q_ASSERT_X(value, "QUaModbusClientWidget", "Value instance must already exist in OPC UA");

	// object column
	auto row  = parent->rowCount();
	auto iObj = new QStandardItem(strValueId);
	parent->setChild(row, (int)Headers::Objects, iObj);
	// set data
	iObj->setData(QVariant::fromValue(QModbusSelectType::QUaModbusValue), QUaModbusClientTree::SelectTypeRole);
	iObj->setData(QVariant::fromValue(value), QUaModbusClientTree::PointerRole);

	// status column
	auto enumError = QMetaEnum::fromType<QModbusError>();
	auto error     = value->lastError();
	auto modError  = error->value().value<QModbusError>();
	auto strError  = QString(enumError.valueToKey(modError));
	auto iErr      = new QStandardItem(strError);
	parent->setChild(row, (int)Headers::Status, iErr);
	// set data
	iErr->setData(QVariant::fromValue(QModbusSelectType::QUaModbusValue), QUaModbusClientTree::SelectTypeRole);
	iErr->setData(QVariant::fromValue(value), QUaModbusClientTree::PointerRole);
	// changes
	QObject::connect(value, &QUaModbusValue::lastErrorChanged, this,
	[iErr, enumError](const QModbusError &error) {
		Q_CHECK_PTR(iErr);
		auto strState = QString(enumError.valueToKey(error));
		iErr->setText(strState);
	});

	// ua delete
	// NOTE : set block as receiver, so callback is not called if block has been deleted
	QObject::connect(value, &QObject::destroyed, block,
	[parent, iObj]() {
		Q_CHECK_PTR(parent);
		Q_CHECK_PTR(iObj);
		parent->removeRows(iObj->index().row(), 1);
	});

	return iObj;
}

void QUaModbusClientTree::saveContentsCsvToFile(const QString & strContents) const
{
	// select file
	QString strSaveFile = QFileDialog::getSaveFileName(const_cast<QUaModbusClientTree*>(this), tr("Save File"),
		QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
		tr("CSV (*.csv *.txt)"));
	// ignore if empty
	if (strSaveFile.isEmpty() || strSaveFile.isNull())
	{
		return;
	}
	// save to file
	QFile file(strSaveFile);
	if (file.open(QIODevice::ReadWrite | QFile::Truncate))
	{
		// write
		QTextStream stream(&file);
		stream << strContents;
	}
	else
	{
		QMessageBox::critical(
			const_cast<QUaModbusClientTree*>(this),
			tr("Error"),
			tr("Error opening file %1 for write operations.").arg(strSaveFile)
		);
	}
	// close file
	file.close();
}

QString QUaModbusClientTree::loadContentsCsvFromFile()
{
	// setup error dialog just in case
	QMessageBox msgBox;
	msgBox.setWindowTitle("Error");
	msgBox.setIcon(QMessageBox::Critical);
	// read from file
	QString strLoadFile = QFileDialog::getOpenFileName(this, tr("Open File"),
		QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
		QObject::trUtf8("CSV (*.csv *.txt)"));
	// validate
	if (strLoadFile.isEmpty())
	{
		return QString();
	}
	QFile fileConfig(strLoadFile);
	// exists
	if (!fileConfig.exists())
	{
		msgBox.setText(tr("File %1 does not exist.").arg(strLoadFile));
		msgBox.exec();
	}
	else if (fileConfig.open(QIODevice::ReadOnly))
	{
		// read
		return fileConfig.readAll();
	}
	else
	{
		msgBox.setText(tr("File %1 could not be opened.").arg(strLoadFile));
		msgBox.exec();
	}
	return QString();
}

void QUaModbusClientTree::displayCsvLoadResult(const QString & strError) const
{
	if (strError.contains("Warning", Qt::CaseInsensitive))
	{
		QMessageBox::warning(
			const_cast<QUaModbusClientTree*>(this),
			tr("Warning"),
			strError
		);
	}
	else if (strError.contains("Error", Qt::CaseInsensitive))
	{
		QMessageBox::critical(
			const_cast<QUaModbusClientTree*>(this),
			tr("Warning"),
			strError
		);
	}
	else if (strError.contains("Success", Qt::CaseInsensitive))
	{
		QMessageBox::information(
			const_cast<QUaModbusClientTree*>(this),
			tr("Information"),
			strError
		);
	}
}
