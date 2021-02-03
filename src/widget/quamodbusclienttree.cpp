#include "quamodbusclienttree.h"
#include "ui_quamodbusclienttree.h"

#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QFileDialog>
#include <QStandardPaths>
#include <QSignalBlocker>

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
#include <QUaModbusDataBlockWidgetEdit>
#include <QUaModbusValueWidgetEdit>

#include <QUaCommonDialog>
#include <QUaLogWidget>

#ifdef QUA_ACCESS_CONTROL
#include <QUaUser>
#include <QUaPermissions>
#include <QUaDockWidgetPerms>
#endif // QUA_ACCESS_CONTROL

template<>
inline QMetaObject::Connection
QUaModelItemTraits::DestroyCallback<QUaNode*, 1>(
	QUaNode* node,
	const std::function<void(void)>& callback)
{
	if (!node)
	{
		return QMetaObject::Connection();
	}
	// handle client list (root)
	auto clientList = qobject_cast<QUaModbusClientList*>(node);
	if (clientList)
	{
		return QObject::connect(clientList, &QUaModbusClientList::aboutToDestroy,
		[callback]() {
			callback();
		});
	}
	// handle client
	auto client = qobject_cast<QUaModbusClient*>(node);
	if (client)
	{
		return QObject::connect(client, &QUaModbusClient::aboutToDestroy,
		[callback]() {
			callback();
		});
	}
	// handle block
	auto block = qobject_cast<QUaModbusDataBlock*>(node);
	if (block)
	{
		return QObject::connect(block, &QUaModbusDataBlock::aboutToDestroy,
		[callback]() {
			callback();
		});
	}
	// handle value
	auto value = qobject_cast<QUaModbusValue*>(node);
	if (value)
	{
		return QObject::connect(value, &QUaModbusValue::aboutToDestroy,
		[callback]() {
			callback();
		});
	}
	// should never reach
	Q_ASSERT(false);
	return QMetaObject::Connection();
	/*
	return QObject::connect(node, &QObject::destroyed,
		[callback]() {
			callback();
		});
	*/
}

template<>
inline QMetaObject::Connection
QUaModelItemTraits::NewChildCallback<QUaNode*, 1>(
	QUaNode* node,
	const std::function<void(QUaNode*)>& callback)
{
	if (!node)
	{
		return QMetaObject::Connection();
	}
	// handle client list (root)
	auto clientList = qobject_cast<QUaModbusClientList*>(node);
	if (clientList)
	{
		return QObject::connect(node, &QUaNode::childAdded,
			[callback](QUaNode* child) {
				// only clients
				auto client = qobject_cast<QUaModbusClient*>(child);
				if (!client)
				{
					return;
				}
				callback(client);
			});
	}
	// handle client
	auto client = qobject_cast<QUaModbusClient*>(node);
	if (client)
	{
		auto blockList = client->dataBlocks();
		return QObject::connect(blockList, &QUaNode::childAdded,
			[callback](QUaNode* child) {
				// only blocks
				auto block = qobject_cast<QUaModbusDataBlock*>(child);
				if (!block)
				{
					return;
				}
				callback(block);
			});
	}
	// handle block
	auto block = qobject_cast<QUaModbusDataBlock*>(node);
	if (block)
	{
		auto valueList = block->values();
		return QObject::connect(valueList, &QUaNode::childAdded,
			[callback](QUaNode* child) {
				// only values
				auto value = qobject_cast<QUaModbusValue*>(child);
				if (!value)
				{
					return;
				}
				callback(value);
			});
	}
	// values have no children
	return QMetaObject::Connection();
}

template<>
inline QList<QUaNode*>
QUaModelItemTraits::GetChildren<QUaNode*, 1>(QUaNode* node)
{
	auto retList = QList<QUaNode*>();
	if (!node)
	{
		return retList;
	}
	// handle client list (root)
	auto clientList = qobject_cast<QUaModbusClientList*>(node);
	if (clientList)
	{
		for (auto client : clientList->clients())
		{
			retList << static_cast<QUaNode*>(client);
		}
		return retList;
	}
	// handle client
	auto client = qobject_cast<QUaModbusClient*>(node);
	if (client)
	{
		auto blockList = client->dataBlocks();
		for (auto block : blockList->blocks())
		{
			retList << static_cast<QUaNode*>(block);
		}
		return retList;
	}
	// handle block
	auto block = qobject_cast<QUaModbusDataBlock*>(node);
	if (block)
	{
		auto valueList = block->values();
		for (auto value : valueList->values())
		{
			retList << static_cast<QUaNode*>(value);
		}
		return retList;
	}
	// values have no children
	return retList;
}

// overload to support default editor (QStyledItemDelegate::setEditorData)
// implement either this or ui->myview->setColumnEditor
// setColumnEditor has preference in case both implemented
template<>
inline bool
QUaModelItemTraits::SetData<QUaNode*, 1>(
	QUaNode* node,
	const int& column,
	const QVariant& value)
{
	Q_UNUSED(column);
	QString strType(node->metaObject()->className());
	// only set value for variables
	if (strType.compare("QUaProperty", Qt::CaseSensitive) != 0 &&
		strType.compare("QUaBaseDataVariable", Qt::CaseSensitive) != 0)
	{
		return false;
	}
	auto var = qobject_cast<QUaBaseVariable*>(node);
	Q_CHECK_PTR(var);
	var->setValue(value);
	return true;
}

/******************************************************************************************************
*/

QUaModbusClientTree::QUaModbusClientTree(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::QUaModbusClientTree)
{
	ui->setupUi(this);
	m_listClients = nullptr;
#ifndef QUA_ACCESS_CONTROL
	ui->pushButtonPerms->setVisible(false);
#else
	m_loggedUser = nullptr;
	m_proxyPerms = nullptr;
	ui->pushButtonPerms->setToolTip(tr(
		"Sets the client list permissions.\n"
		"Read permissions do nothing. To disallow showing the client tree, use the dock's permissions."
		"Write permissions control if clients can be added or removed."
	));
#endif // QUA_ACCESS_CONTROL
	m_strLastPathUsed = QString();
	m_colorLogError = QColor("#8E2F1C");
	m_colorLogWarn = QColor("#766B0F");
	m_colorLogInfo = QColor("#265EB6");
	this->setupTreeContextMenu();
	this->setupImportButton();
	this->setupExportButton();
	this->setupFilterWidgets();
	// setup params sort filter
	// NOTE : do not set proxy's source model until source columns have been defined
	m_proxyModbus.setFilterAcceptsRow(
	[this](int sourceRow, const QModelIndex & sourceParent) {
		// get pointer to base class
		auto index = m_modelModbus.index(sourceRow, 0, sourceParent);
		if (!index.isValid())
		{
			return false;
		}
		auto node = m_modelModbus.nodeFromIndex(index);
		if (!node)
		{
			return false;
		}
		auto type =
			qobject_cast<QUaModbusClient*>   (node) ? QModbusSelectType::QUaModbusClient    :
			qobject_cast<QUaModbusDataBlock*>(node) ? QModbusSelectType::QUaModbusDataBlock :
			qobject_cast<QUaModbusValue*>    (node) ? QModbusSelectType::QUaModbusValue     :
			QModbusSelectType::Invalid;

		// filter type
		Qt::CaseSensitivity sensitive = ui->checkBoxCase->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive;
		bool show = false;
		ComboOpts typeFilter = ui->comboBoxFilterType->currentData().value<ComboOpts>();
		switch (typeFilter)
		{
		case QUaModbusClientTree::ComboOpts::Clients:
			show = true;
			if (type == QModbusSelectType::QUaModbusClient)
			{
				show = index.data().toString().contains(ui->lineEditFilterText->text(), sensitive);
			}
			break;
		case QUaModbusClientTree::ComboOpts::Blocks:
			show = true;
			if (type == QModbusSelectType::QUaModbusDataBlock)
			{
				show = index.data().toString().contains(ui->lineEditFilterText->text(), sensitive);
			}
			break;
		case QUaModbusClientTree::ComboOpts::Values:
			show = true;
			if (type == QModbusSelectType::QUaModbusValue)
			{
				show = index.data().toString().contains(ui->lineEditFilterText->text(), sensitive);
			}
			break;
		default:
			show = false;
			break;
		}
#ifdef QUA_ACCESS_CONTROL
		// filter permissions
		QUaPermissions * perms = nullptr;
		switch (type)
		{
		case QModbusSelectType::QUaModbusClient:
			{
				auto client = qobject_cast<QUaModbusClient*>(node);
				Q_CHECK_PTR(client);
				perms = client->permissionsObject();
			}
			break;
		case QModbusSelectType::QUaModbusDataBlock:
			{
				auto block = qobject_cast<QUaModbusDataBlock*>(node);
				Q_CHECK_PTR(block);
				perms = block->permissionsObject();
			}
			break;
		case QModbusSelectType::QUaModbusValue:
			{
				auto value = qobject_cast<QUaModbusValue*>(node);
				Q_CHECK_PTR(value);
				perms = value->permissionsObject();
			}
			break;
		default:
			Q_ASSERT(false);
			break;
		}
		bool canRead = !m_loggedUser ? false : !perms ? true : perms->canUserRead(m_loggedUser);
		show = show && canRead;
#endif // QUA_ACCESS_CONTROL
		// return combination
		return show;
	});
	// setup params tree
	// NOTE : do not set tree's proxy model until source columns have been defined
	ui->treeViewModbus->setAlternatingRowColors(true);
	ui->treeViewModbus->setSortingEnabled(true);
	ui->treeViewModbus->sortByColumn((int)Headers::Objects, Qt::SortOrder::AscendingOrder);
	ui->treeViewModbus->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->treeViewModbus->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->treeViewModbus->setEditTriggers(QAbstractItemView::NoEditTriggers);
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
	// set list as root node
	m_listClients = listClients;
	m_modelModbus.setRootNode(m_listClients);
	// setup model column data sources
	int colObjects = static_cast<int>(Headers::Objects);
	m_modelModbus.setColumnDataSource(colObjects, QString(QMetaEnum::fromType<Headers>().valueToKey(colObjects)),
	[this](QUaNode* node, const Qt::ItemDataRole &role) -> QVariant {
		if (role == Qt::DisplayRole)
		{
			return node->displayName().text();
		}
		if (role == Qt::DecorationRole)
		{
			auto icon =
				qobject_cast<QUaModbusClient*>   (node) ? 
				qobject_cast<QUaModbusTcpClient*>      (node) ? this->iconClientTcp() :
				qobject_cast<QUaModbusRtuSerialClient*>(node) ? this->iconClientSerial() : QIcon() :
				qobject_cast<QUaModbusDataBlock*>(node) ? this->iconBlock() :
				qobject_cast<QUaModbusValue*>    (node) ? this->iconValue() :
				QIcon();
			return icon;
		}
		return QVariant();
	}/* second callback is only necessary for data that changes */);
	int colStatus = static_cast<int>(Headers::Status);
	m_modelModbus.setColumnDataSource(colStatus, QString(QMetaEnum::fromType<Headers>().valueToKey(colStatus)),
	[](QUaNode* node, const Qt::ItemDataRole& role) -> QVariant {
		if (role == Qt::DisplayRole)
		{
			auto client = qobject_cast<QUaModbusClient*>(node);
			if (client)
			{
				auto enumState = QMetaEnum::fromType<QModbusState>();
				auto state     = client->state();
				auto modState  = state->value().value<QModbusState>();
				auto strState  = QString(enumState.valueToKey(modState));
				auto enumError = QMetaEnum::fromType<QModbusError>();
				auto error     = client->lastError();
				auto modError  = error->value().value<QModbusError>();
				auto strError  = QString(enumError.valueToKey(modError));
				return strState + " | " + strError;
			}
			auto block = qobject_cast<QUaModbusDataBlock*>(node);
			if (block)
			{
				auto enumError = QMetaEnum::fromType<QModbusError>();
				auto error     = block->lastError();
				auto modError  = error->value().value<QModbusError>();
				auto strError  = QString(enumError.valueToKey(modError));
				return strError;
			}
			auto value = qobject_cast<QUaModbusValue*>(node);
			if (value)
			{
				auto enumError = QMetaEnum::fromType<QModbusError>();
				auto error     = value->lastError();
				auto modError  = error->value().value<QModbusError>();
				auto strError  = QString(enumError.valueToKey(modError));
				return strError;
			}
		}
		return QVariant();
	},
	[](QUaNode* node, std::function<void(void)> callback) -> QList<QMetaObject::Connection> {
		QList<QMetaObject::Connection> retConns;
		auto client = qobject_cast<QUaModbusClient*>(node);
		if (client)
		{
			retConns <<
			QObject::connect(client, &QUaModbusClient::stateChanged,
			[callback](QModbusState state) {
				Q_UNUSED(state);
				callback();
			});
			retConns << 
			QObject::connect(client, &QUaModbusClient::lastErrorChanged,
			[callback](const QModbusError &error) {
				Q_UNUSED(error);
				callback();
			});
			return retConns;
		}
		auto block = qobject_cast<QUaModbusDataBlock*>(node);
		if (block)
		{
			retConns << QObject::connect(block, &QUaModbusDataBlock::lastErrorChanged,
			[callback](const QModbusError &error) {
				Q_UNUSED(error);
				callback();
			});
			return retConns;
		}
		auto value = qobject_cast<QUaModbusValue*>(node);
		if (value)
		{
			retConns <<
			QObject::connect(value, &QUaModbusValue::lastErrorChanged,
			[callback](const QModbusError &error) {
				Q_UNUSED(error);
				callback();
			});
			return retConns;
		}
		return retConns;
	});

	// NOTE : now is the good moment to set the proxy's source and the tree's proxy model
	m_proxyModbus.setSourceModel(&m_modelModbus);
	ui->treeViewModbus->setModel(&m_proxyModbus);

	// setup tree interactions
	QObject::connect(ui->treeViewModbus->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
	[this](const QModelIndex &current, const QModelIndex &previous) {
		auto indexPrev = m_proxyModbus.mapToSource(previous);
		auto indexCurr = m_proxyModbus.mapToSource(current);
		// previous
		QModbusSelectType typePrev = QModbusSelectType::Invalid;
		QUaNode * nodePrev = nullptr;
		if (indexPrev.isValid())
		{
			nodePrev = m_modelModbus.nodeFromIndex(indexPrev);
			typePrev =
				qobject_cast<QUaModbusClient*>   (nodePrev) ? QModbusSelectType::QUaModbusClient    :
				qobject_cast<QUaModbusDataBlock*>(nodePrev) ? QModbusSelectType::QUaModbusDataBlock :
				qobject_cast<QUaModbusValue*>    (nodePrev) ? QModbusSelectType::QUaModbusValue     :
				QModbusSelectType::Invalid;
		}
		// current
		QModbusSelectType typeCurr = QModbusSelectType::Invalid;
		QUaNode * nodeCurr = nullptr;
		if (indexCurr.isValid())
		{
			nodeCurr = m_modelModbus.nodeFromIndex(indexCurr);
			typeCurr = 
				qobject_cast<QUaModbusClient*>   (nodeCurr) ? QModbusSelectType::QUaModbusClient    :
				qobject_cast<QUaModbusDataBlock*>(nodeCurr) ? QModbusSelectType::QUaModbusDataBlock :
				qobject_cast<QUaModbusValue*>    (nodeCurr) ? QModbusSelectType::QUaModbusValue     :
				QModbusSelectType::Invalid;
		}
		// emit
		emit this->nodeSelectionChanged(nodePrev, typePrev, nodeCurr, typeCurr);
	});
	// emit on double click
	QObject::connect(ui->treeViewModbus, &QAbstractItemView::doubleClicked, this,
	[this](const QModelIndex &proxyIndex) {
		auto index = m_proxyModbus.mapToSource(proxyIndex);
		if (!index.isValid())
		{
			return;
		}
		auto node = m_modelModbus.nodeFromIndex(index);
		if (!node)
		{
			return;
		}
		auto type = 
			qobject_cast<QUaModbusClient*>   (node) ? QModbusSelectType::QUaModbusClient    :
			qobject_cast<QUaModbusDataBlock*>(node) ? QModbusSelectType::QUaModbusDataBlock :
			qobject_cast<QUaModbusValue*>    (node) ? QModbusSelectType::QUaModbusValue     :
			QModbusSelectType::Invalid;
		switch (type)
		{
		case QModbusSelectType::QUaModbusClient:
			{
				auto client = qobject_cast<QUaModbusClient*>(node);
				emit this->clientDoubleClicked(client);
			}
			break;
		case QModbusSelectType::QUaModbusDataBlock:
			{
				auto block = qobject_cast<QUaModbusDataBlock*>(node);
				emit this->blockDoubleClicked(block);
			}
			break;
		case QModbusSelectType::QUaModbusValue:
			{
				auto value = qobject_cast<QUaModbusValue*>(node);
				emit this->valueDoubleClicked(value);
			}
			break;
		default:
			Q_ASSERT(false);
			break;
		}
	});
#ifdef QUA_ACCESS_CONTROL
	QObject::connect(ui->pushButtonPerms, &QPushButton::clicked, listClients,
	[this, listClients]() {
		// NOTE : call ::setupPermissionsModel first to set m_proxyPerms
		Q_CHECK_PTR(m_proxyPerms);
		// create permissions widget
		auto permsWidget = new QUaDockWidgetPerms;
		// configure perms widget combo
		permsWidget->setComboModel(m_proxyPerms);
		permsWidget->setPermissions(listClients->permissionsObject());
		// dialog
		QUaModbusClientDialog dialog(this);
		dialog.setWindowTitle(tr("Modbus Permissions"));
		dialog.setWidget(permsWidget);
		// exec dialog
		int res = dialog.exec();
		if (res != QDialog::Accepted)
		{
			return;
		}
		// read permissions and set them for layout list
		auto perms = permsWidget->permissions();
		perms ? listClients->setPermissionsObject(perms) : listClients->clearPermissions();
		// update widgets
		this->on_loggedUserChanged(m_loggedUser);
	});
	// handle permission changes
	m_modelModbus.connectNodeAddedCallback(this, 
	[this](QUaNode* node, const QModelIndex &index) {
		Q_UNUSED(node);
		Q_UNUSED(index);
		auto object = qobject_cast<QUaBaseObjectProtected*>(node);
		if (object)
		{
			QObject::connect(object, &QUaBaseObjectProtected::permissionsObjectChanged, this,
			[this]() {
				// update filter to take permissions into account
				m_proxyModbus.resetFilter();
			});
		}
	});
#endif // QUA_ACCESS_CONTROL
}

#ifdef QUA_ACCESS_CONTROL
void QUaModbusClientTree::setupPermissionsModel(QSortFilterProxyModel * proxyPerms)
{
	m_proxyPerms = proxyPerms;
	Q_CHECK_PTR(m_proxyPerms);
}

void QUaModbusClientTree::on_loggedUserChanged(QUaUser * user)
{
	m_loggedUser = user;
	// update filter to take permissions into account
	m_proxyModbus.resetFilter();
	// show/hide add client button depending on list perms
	auto permsList    = m_listClients->permissionsObject();
	auto canWriteList = !permsList ? true : permsList->canUserWrite(m_loggedUser);
	QString strToolTip = canWriteList ?
		tr("") :
		tr("Do not have permissions.");
	ui->pushButtonAddClient->setEnabled(canWriteList);
	ui->toolButtonImport   ->setEnabled(canWriteList);
	ui->pushButtonClear    ->setEnabled(canWriteList);
	ui->pushButtonPerms    ->setVisible(canWriteList); // NOTE : only hide this one
	// tooltips
	ui->pushButtonAddClient->setToolTip(strToolTip);
	ui->toolButtonImport   ->setToolTip(strToolTip);
	ui->pushButtonClear    ->setToolTip(strToolTip);
}
#endif // QUA_ACCESS_CONTROL

void QUaModbusClientTree::setExpanded(const bool & expanded)
{
	this->expandRecursivelly(ui->treeViewModbus->rootIndex(), expanded);
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

QItemSelectionModel * QUaModbusClientTree::selectionModel() const
{
	return ui->treeViewModbus->selectionModel();
}

void QUaModbusClientTree::exportAllCsv(const QString & strBaseName)
{
	// NOTE : suffix must not collide with any other from the main aaplication
	QFileInfo infoBaseName = QFileInfo(strBaseName);
	QString strBase        = infoBaseName.baseName();
	QString strSuff        = infoBaseName.completeSuffix();
	QString strPath        = infoBaseName.absolutePath();
	QString strClientsName = QString("%1/%2%3.%4").arg(strPath).arg(strBase).arg("_clients").arg(strSuff);
	QString strBlocksName  = QString("%1/%2%3.%4").arg(strPath).arg(strBase).arg("_blocks" ).arg(strSuff);
	QString strValuesName  = QString("%1/%2%3.%4").arg(strPath).arg(strBase).arg("_values" ).arg(strSuff);
	this->saveContentsCsvToFile(m_listClients->csvClients(), strClientsName);
	this->saveContentsCsvToFile(m_listClients->csvBlocks (), strBlocksName );
	this->saveContentsCsvToFile(m_listClients->csvValues (), strValuesName );
}

void QUaModbusClientTree::on_pushButtonClear_clicked()
{
	// ask user if create new config
	auto res = QMessageBox::question(
		this,
		tr("Delete All Clients Confirmation"),
		tr("Are you sure you want to delete all clients?\nAll their blocks and values will also be deleted."),
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
	m_listClients->clear();
}

void QUaModbusClientTree::setupTreeContextMenu()
{
	ui->treeViewModbus->setContextMenuPolicy(Qt::CustomContextMenu);
	QObject::connect(ui->treeViewModbus, &QTreeView::customContextMenuRequested, this, 
	[this](const QPoint &point) {
		QModelIndex index = ui->treeViewModbus->indexAt(point);
        QMenu contextMenu(ui->treeViewModbus);
		if (!index.isValid())
		{
            contextMenu.addAction(m_iconAdd, tr("Add Client"), this,
            [this]() {
                QUaModbusClientWidgetEdit * widgetNewClient = new QUaModbusClientWidgetEdit;
                QUaModbusClientDialog dialog(this);
                dialog.setWindowTitle(tr("New Modbus Client"));
                // NOTE : dialog takes ownershit
                dialog.setWidget(widgetNewClient);
                // NOTE : call in own method to we can recall it if fails
                this->showNewClientDialog(dialog);
            });
            // exec
            contextMenu.exec(ui->treeViewModbus->viewport()->mapToGlobal(point));
			return;
        }
		// specific
		auto indexSource = m_proxyModbus.mapToSource(index);
		auto node = m_modelModbus.nodeFromIndex(indexSource);
		auto type = 
			qobject_cast<QUaModbusClient*>   (node) ? QModbusSelectType::QUaModbusClient    :
			qobject_cast<QUaModbusDataBlock*>(node) ? QModbusSelectType::QUaModbusDataBlock :
			qobject_cast<QUaModbusValue*>    (node) ? QModbusSelectType::QUaModbusValue     :
			QModbusSelectType::Invalid;
		switch (type)
		{
		case QModbusSelectType::QUaModbusClient:
			// for clients only
			{
				// expand/collapse
				contextMenu.addAction(m_iconExpand, tr("Expand"), this,
				[this, index]() {
					this->expandRecursivelly(index, true);
				});
				contextMenu.addAction(m_iconCollapse, tr("Collapse"), this,
				[this, index]() {
					this->expandRecursivelly(index, false);
				});
				auto client = qobject_cast<QUaModbusClient*>(node);
				contextMenu.addSeparator();
				// connect
				bool isConnected = client->getState() == QModbusState::ConnectedState;
				contextMenu.addAction(m_iconConnect, isConnected? tr("Disconnect Client") : tr("Connect Client"), this,
				[client]() {
					bool isConnected = client->getState() == QModbusState::ConnectedState;
					isConnected ? client->disconnectDevice() : client->connectDevice();
				});
				contextMenu.addSeparator();
				// blocks
				contextMenu.addAction(m_iconAdd, tr("Add Block"), this,
				[this, client]() {
					Q_CHECK_PTR(client);
					// use block edit widget
					QUaModbusDataBlockWidgetEdit* widgetNewBlock = new QUaModbusDataBlockWidgetEdit;
					QUaModbusClientDialog dialog(this);
					dialog.setWindowTitle(tr("New Modbus Block"));
					// NOTE : dialog takes ownershit
					dialog.setWidget(widgetNewBlock);
					// NOTE : call in own method to we can recall it if fails
					this->showNewBlockDialog(client, dialog);
				});
				contextMenu.addAction(m_iconClear, tr("Clear Blocks"), this,
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
					// clear
					client->dataBlocks()->clear();
				});
				contextMenu.addSeparator();
				// delete client
				contextMenu.addAction(m_iconDelete, tr("Delete"), this,
				[this, client]() {
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
			}
			break;
		case QModbusSelectType::QUaModbusDataBlock:
			// for blocks only
			{
				contextMenu.addAction(m_iconExpand, tr("Expand"), this,
				[this, index]() {
					this->expandRecursivelly(index, true);
				});
				contextMenu.addAction(m_iconCollapse, tr("Collapse"), this,
				[this, index]() {
					this->expandRecursivelly(index, false);
				});
				auto block = qobject_cast<QUaModbusDataBlock*>(node);
				contextMenu.addSeparator();
				// values
				contextMenu.addAction(m_iconAdd, tr("Add Value"), this,
				[this, block]() {
					Q_CHECK_PTR(block);
					// use value edit widget
					QUaModbusValueWidgetEdit* widgetNewValue = new QUaModbusValueWidgetEdit;
					QUaModbusClientDialog dialog(this);
					dialog.setWindowTitle(tr("New Modbus Value"));
					// NOTE : dialog takes ownershit
					dialog.setWidget(widgetNewValue);
					// NOTE : call in own method to we can recall it if fails
					this->showNewValueDialog(block, dialog);
				});
				contextMenu.addAction(m_iconClear, tr("Clear Values"), this,
				[this, block]() {
					Q_CHECK_PTR(block);
					// are you sure?
					auto res = QMessageBox::question(
						this,
						tr("Delete All Values Confirmation"),
						tr("Are you sure you want to delete all values for block %1?\n").arg(block->browseName().name()),
						QMessageBox::StandardButton::Ok,
						QMessageBox::StandardButton::Cancel
					);
					if (res != QMessageBox::StandardButton::Ok)
					{
						return;
					}
					// clear
					block->values()->clear();
				});
				contextMenu.addSeparator();
				// delete block
				contextMenu.addAction(m_iconDelete, tr("Delete"), this,
				[this, block]() {
					Q_CHECK_PTR(block);
					// are you sure?
					auto res = QMessageBox::question(
						this,
						tr("Delete Block Confirmation"),
						tr("Deleting block %1 will also delete all its Values.\nWould you like to delete block %1?").arg(block->browseName().name()),
						QMessageBox::StandardButton::Ok,
						QMessageBox::StandardButton::Cancel
					);
					if (res != QMessageBox::StandardButton::Ok)
					{
						return;
					}
					// delete
					block->remove();
					// NOTE : removed from tree on &QObject::destroyed callback
				});
			}
			break;
		case QModbusSelectType::QUaModbusValue:
			// for values only
			{
				auto value = qobject_cast<QUaModbusValue*>(node);
				contextMenu.addSeparator();
				// delete value
				contextMenu.addAction(m_iconDelete, tr("Delete"), this,
				[this, value]() {
					Q_CHECK_PTR(value);
					// are you sure?
					auto res = QMessageBox::question(
						this,
						tr("Delete Value Confirmation"),
						tr("Would you like to delete value %1?").arg(value->browseName().name()),
						QMessageBox::StandardButton::Ok,
						QMessageBox::StandardButton::Cancel
					);
					if (res != QMessageBox::StandardButton::Ok)
					{
						return;
					}
					// delete
					value->remove();
					// NOTE : removed from tree on &QObject::destroyed callback
				});
			}
			break;
		default:
			break;
		}
		// exec
		contextMenu.exec(ui->treeViewModbus->viewport()->mapToGlobal(point));
	});
}

void QUaModbusClientTree::setupImportButton()
{
	ui->toolButtonImport->setMaximumHeight(ui->pushButtonClear->sizeHint().height());
	ui->toolButtonImport->setMinimumHeight(ui->pushButtonClear->sizeHint().height());
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
		// NOTE : block signals to avoid calling currentRowChanged repetitively
		//        it works with queued because it seems showing a dialog forces event processing
		const QSignalBlocker blocker(ui->treeViewModbus->selectionModel());
		QQueue<QUaLog> errorLogs = m_listClients->setCsvClients(strContents);
		this->displayCsvLoadResult(errorLogs);
	});
	importMenu->addAction(tr("Blocks"), this, 
	[this](){
		QString strContents = loadContentsCsvFromFile();
		if (strContents.isEmpty())
		{
			return;
		}
		// NOTE : block signals to avoid calling currentRowChanged repetitively
		//        it works with queued because it seems showing a dialog forces event processing
		const QSignalBlocker blocker(ui->treeViewModbus->selectionModel());
		QQueue<QUaLog> errorLogs = m_listClients->setCsvBlocks(strContents);
		this->displayCsvLoadResult(errorLogs);
	});
	importMenu->addAction(tr("Values"), this, 
	[this](){
		QString strContents = loadContentsCsvFromFile();
		if (strContents.isEmpty())
		{
			return;
		}
		// NOTE : block signals to avoid calling currentRowChanged repetitively
		//        it works with queued because it seems showing a dialog forces event processing
		const QSignalBlocker blocker(ui->treeViewModbus->selectionModel());
		QQueue<QUaLog> errorLogs = m_listClients->setCsvValues(strContents);
		this->displayCsvLoadResult(errorLogs);
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
	ui->toolButtonExport->setMaximumHeight(ui->pushButtonClear->sizeHint().height());
	ui->toolButtonExport->setMinimumHeight(ui->pushButtonClear->sizeHint().height());
	ui->toolButtonExport->setPopupMode(QToolButton::MenuButtonPopup);
	// menu
	auto exportMenu = new QMenu(ui->toolButtonExport);
	// all
	exportMenu->addAction(tr("All"), this,
	[this]() {
		this->exportAllCsv();
	});
	exportMenu->addSeparator();
	// individual
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

void QUaModbusClientTree::setupFilterWidgets()
{
	// initially hidden
	this->setFilterVisible(false);
	// setup combobox options
	ui->comboBoxFilterType->addItem(tr("Clients"), QVariant::fromValue(ComboOpts::Clients));
	ui->comboBoxFilterType->addItem(tr("Blocks" ), QVariant::fromValue(ComboOpts::Blocks) );
	ui->comboBoxFilterType->addItem(tr("Values" ), QVariant::fromValue(ComboOpts::Values) );
	// set current combo index as values
	int indexAny = ui->comboBoxFilterType->findData(QVariant::fromValue(ComboOpts::Values));
	Q_ASSERT(indexAny >= 0);
	ui->comboBoxFilterType->setCurrentIndex(indexAny);
}

void QUaModbusClientTree::expandRecursivelly(const QModelIndex & index, const bool & expand)
{
	// first children
	int childRow = 0;
	auto childIndex = m_proxyModbus.index(childRow, 0, index);
	while (childIndex.isValid())
	{
		this->expandRecursivelly(childIndex, expand);
		childIndex = m_proxyModbus.index(++childRow, 0, index);
	}
	// finally parent
	ui->treeViewModbus->setExpanded(index, expand);
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
		// NOTE : removed, because is not the same "keep connecting after failure" than
		//        "auto connect at startup", which should be handled at higher level in the app
		/*
		if (cliTcp->getKeepConnecting())
		{
			cliTcp->connectDevice();
		}
		*/
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
		// NOTE : removed, because is not the same "keep connecting after failure" than
		//        "auto connect at startup", which should be handled at higher level in the app
		/*
		if (cliSerial->getKeepConnecting())
		{
			cliSerial->connectDevice();
		}
		*/
	}
	break;
	case QModbusClientType::Invalid:
	default:
		Q_ASSERT(false);
		break;
	}
	// NOTE : new client is added to GUI using OPC UA events 
}

void QUaModbusClientTree::saveContentsCsvToFile(const QString & strContents, const QString &strFileName/* = ""*/)
{
	// select file
	QString strSaveFile = !strFileName.isEmpty() ? strFileName :
		QFileDialog::getSaveFileName(this, tr("Save File"),
		m_strLastPathUsed.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) : m_strLastPathUsed,
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
		// save last path used
		m_strLastPathUsed = QFileInfo(file).absoluteFilePath();
		// write
		QTextStream stream(&file);
		stream << strContents;
	}
	else
	{
		QMessageBox::critical(
			this,
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
		m_strLastPathUsed.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) : m_strLastPathUsed,
		tr("CSV (*.csv *.txt)"));
	// validate
	if (strLoadFile.isEmpty())
	{
		return QString();
	}
	QFile file(strLoadFile);
	// exists
	if (!file.exists())
	{
		msgBox.setText(tr("File %1 does not exist.").arg(strLoadFile));
		msgBox.exec();
	}
	else if (file.open(QIODevice::ReadOnly))
	{
		// save last path used
		m_strLastPathUsed = QFileInfo(file).absoluteFilePath();
		// read
		return file.readAll();
	}
	else
	{
		msgBox.setText(tr("File %1 could not be opened.").arg(strLoadFile));
		msgBox.exec();
	}
	return QString();
}

void QUaModbusClientTree::displayCsvLoadResult(QQueue<QUaLog>& errorLogs)
{
	if (errorLogs.isEmpty())
	{
		return;
	}
	// setup log widget
	auto logWidget = new QUaLogWidget;
	logWidget->setFilterVisible(false);
	logWidget->setSettingsVisible(false);
	logWidget->setClearVisible(false);
	logWidget->setColumnVisible(QUaLogWidget::Columns::Timestamp, false);
	logWidget->setColumnVisible(QUaLogWidget::Columns::Category, false);
	logWidget->setLevelColor(QUaLogLevel::Error, QBrush(m_colorLogError));
	logWidget->setLevelColor(QUaLogLevel::Warning, QBrush(m_colorLogWarn));
	logWidget->setLevelColor(QUaLogLevel::Info, QBrush(m_colorLogInfo));
	while (errorLogs.count() > 0)
	{
		logWidget->addLog(errorLogs.dequeue());
	}
	// NOTE : dialog takes ownershit
	QUaCommonDialog dialog(this);
	dialog.setWindowTitle(tr("CSV Import Issues"));
	dialog.setWidget(logWidget);
	dialog.clearButtons();
	dialog.addButton(tr("Close"), QDialogButtonBox::ButtonRole::AcceptRole);
	dialog.exec();
}

void QUaModbusClientTree::exportAllCsv()
{
	// ask for base name
	QString strBaseName = QFileDialog::getSaveFileName(this, tr("Select Base File Name"),
		m_strLastPathUsed.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) : m_strLastPathUsed,
		tr("CSV (*.csv *.txt)"));
	// ignore if empty
	if (strBaseName.isEmpty() || strBaseName.isNull())
	{
		return;
	}
	// save all
	this->exportAllCsv(strBaseName);
}

bool QUaModbusClientTree::isFilterVisible() const
{
	return ui->frameFilter->isEnabled();
}

void QUaModbusClientTree::setFilterVisible(const bool & isVisible)
{
	ui->frameFilter->setEnabled(isVisible);
	ui->frameFilter->setVisible(isVisible);
	if (!isVisible)
	{
		// set current combo index as values
		int indexAny = ui->comboBoxFilterType->findData(QVariant::fromValue(ComboOpts::Values));
		if (indexAny >= 0)
		{
			ui->comboBoxFilterType->setCurrentIndex(indexAny);
		}
		// clear filter text
		ui->lineEditFilterText->setText("");
	}
}

void QUaModbusClientTree::showNewBlockDialog(QUaModbusClient* client, QUaModbusClientDialog& dialog)
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

void QUaModbusClientTree::showNewValueDialog(QUaModbusDataBlock* block, QUaModbusClientDialog& dialog)
{
	Q_CHECK_PTR(block);
	int res = dialog.exec();
	if (res != QDialog::Accepted)
	{
		return;
	}
	// get new client type
	auto widgetNewValue = qobject_cast<QUaModbusValueWidgetEdit*>(dialog.widget());
	Q_CHECK_PTR(widgetNewValue);
	// get data from widget
	auto strValueId = widgetNewValue->id();
	// check
	auto listValues = block->values();
	QString strError = listValues->addValue(strValueId);
	if (strError.contains("Error", Qt::CaseInsensitive))
	{
		QMessageBox::critical(this, tr("New Value Error"), strError, QMessageBox::StandardButton::Ok);
		this->showNewValueDialog(block, dialog);
		return;
	}
	// set properties
	auto value = listValues->browseChild<QUaModbusValue>(strValueId);
	Q_CHECK_PTR(value);
	value->setType(widgetNewValue->type());
	value->setAddressOffset(widgetNewValue->offset());
	// NOTE : new value is added to tree using OPC UA events 
}

void QUaModbusClientTree::on_checkBoxFilter_toggled(bool checked)
{
	this->setFilterVisible(checked);
}

void QUaModbusClientTree::on_comboBoxFilterType_currentIndexChanged(int index)
{
	Q_UNUSED(index);
	m_proxyModbus.resetFilter();
}

void QUaModbusClientTree::on_lineEditFilterText_textChanged(const QString &arg1)
{
	Q_UNUSED(arg1);
	m_proxyModbus.resetFilter();
}

QIcon QUaModbusClientTree::iconClientTcp() const
{
	return m_iconClientTcp;
}

void QUaModbusClientTree::setIconClientTcp(const QIcon & icon)
{
	m_iconClientTcp = icon;
}

QIcon QUaModbusClientTree::iconClientSerial() const
{
	return m_iconClientSerial;
}

void QUaModbusClientTree::setIconClientSerial(const QIcon & icon)
{
	m_iconClientSerial = icon;
}

QIcon QUaModbusClientTree::iconBlock() const
{
	return m_iconBlock;
}

void QUaModbusClientTree::setIconBlock(const QIcon & icon)
{
	m_iconBlock = icon;
}

QIcon QUaModbusClientTree::iconValue() const
{
	return m_iconValue;
}

void QUaModbusClientTree::setIconValue(const QIcon & icon)
{
	m_iconValue = icon;
}

QIcon QUaModbusClientTree::iconExpand() const
{
	return m_iconExpand;
}

void QUaModbusClientTree::setIconExpand(const QIcon& icon)
{
	m_iconExpand = icon;
}

QIcon QUaModbusClientTree::iconCollapse() const
{
	return m_iconCollapse;
}

void QUaModbusClientTree::setIconCollapse(const QIcon& icon)
{
	m_iconCollapse = icon;
}

QIcon QUaModbusClientTree::iconAdd() const
{
	return m_iconAdd;
}

void QUaModbusClientTree::setIconAdd(const QIcon& icon)
{
	m_iconAdd = icon;
}

QIcon QUaModbusClientTree::iconDelete() const
{
	return m_iconDelete;
}

void QUaModbusClientTree::setIconDelete(const QIcon& icon)
{
	m_iconDelete = icon;
}

QIcon QUaModbusClientTree::iconClear() const
{
	return m_iconClear;
}

void QUaModbusClientTree::setIconClear(const QIcon& icon)
{
	m_iconClear = icon;
}

QIcon QUaModbusClientTree::iconConnect() const
{
	return m_iconConnect;
}

void QUaModbusClientTree::setIconConnect(const QIcon& icon)
{
	m_iconConnect = icon;
}

QColor QUaModbusClientTree::colorLogError() const
{
	return m_colorLogError;
}

void QUaModbusClientTree::setColorLogError(const QColor& color)
{
	m_colorLogError = color;
}

QColor QUaModbusClientTree::colorLogWarn() const
{
	return m_colorLogWarn;
}

void QUaModbusClientTree::setColorLogWarn(const QColor& color)
{
	m_colorLogWarn = color;
}

QColor QUaModbusClientTree::colorLogInfo() const
{
	return m_colorLogInfo;
}

void QUaModbusClientTree::setColorLogInfo(const QColor& color)
{
	m_colorLogInfo = color;
}

void QUaModbusClientTree::on_checkBoxCase_toggled(bool checked)
{
	Q_UNUSED(checked);
	m_proxyModbus.resetFilter();
}
