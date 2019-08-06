#ifndef QUAMODBUSDOCKWIDGETS_H
#define QUAMODBUSDOCKWIDGETS_H

#include <QUaAcDocking>

// NOTE : might be necessary to forward declare and move to T.cpp
#include <QUaAccessControl>
#include <QUaUser>
#include <QUaRole>
#include <QUaPermissions>
// NOTE : might be necessary to forward declare and move to T.cpp
#include <QUaModbusClientList>
#include <QUaModbusClient>
#include <QUaModbusTcpClient>
#include <QUaModbusRtuSerialClient>
#include <QUaModbusDataBlock>
#include <QUaModbusValue>
#include <QUaModbusClientTree>
#include <QUaModbusClientWidget>
#include <QUaModbusDataBlockWidget>
#include <QUaModbusValueWidget>

template <class T>
class QUaModbusDockWidgets : public QObject
{
public:
    explicit QUaModbusDockWidgets(T *parent = nullptr);

	// NOTE : all public methods are T requirements

	void updateWidgetsPermissions();
	void clearWidgets();
	void closeConfig();

	// XML import / export
	QDomElement toDomElement(QDomDocument & domDoc) const;
	void        fromDomElement(QDomElement  & domElem, QString &strError);

	const static QString m_strXmlName;

private:
	T * m_thiz;

	// modbus widgets
	QUaModbusClientTree      * m_modbusTreeWidget;
	QUaModbusClientWidget    * m_clientWidget;
	QUaModbusDataBlockWidget * m_blockWidget ;
	QUaModbusValueWidget	 * m_valueWidget ; 
	// to be able to update permissions
	QUaModbusClient    * m_client;
	QUaModbusDataBlock * m_block ;
	QUaModbusValue     * m_valueCurr ;

	void createModbusWidgetsDocks();
	void setupModbusTreeWidget();

	void bindClientWidget(QUaModbusClient    * client);
	void bindBlockWidget (QUaModbusDataBlock * block );
	void bindValueWidget (QUaModbusValue     * value );

	void updateClientWidgetPermissions();
	void updateBlockWidgetPermissions();
	void updateValueWidgetPermissions();

	// helpers
	QUaAcDocking * getDockManager() const;

	const static QString m_strModbusTree;
	const static QString m_strModbusClients;
	const static QString m_strModbusBlocks;
	const static QString m_strModbusValues;

	static QList<QString> m_listWidgetNames;
};

template <class T>
const QString QUaModbusDockWidgets<T>::m_strXmlName = "QUaModbusDockWidgets";

template <class T>
const QString QUaModbusDockWidgets<T>::m_strModbusTree = "Modbus Tree";

template <class T>
const QString QUaModbusDockWidgets<T>::m_strModbusClients = "Modbus Client Edit";

template <class T>
const QString QUaModbusDockWidgets<T>::m_strModbusBlocks = "Modbus DataBlock Edit";

template <class T>
const QString QUaModbusDockWidgets<T>::m_strModbusValues = "Modbus Value Edit";

// create list to iterate
template <class T>
QList<QString> QUaModbusDockWidgets<T>::m_listWidgetNames;

template <class T>
inline QUaModbusDockWidgets<T>::QUaModbusDockWidgets(T *parent) : QObject(parent)
{
	Q_CHECK_PTR(parent);
	m_thiz   = parent;
	m_client = nullptr;
	m_block  = nullptr;
	m_valueCurr  = nullptr;
	//
	QUaModbusDockWidgets<T>::m_listWidgetNames = QList<QString>()
		<< QUaModbusDockWidgets<T>::m_strModbusTree
		<< QUaModbusDockWidgets<T>::m_strModbusClients
		<< QUaModbusDockWidgets<T>::m_strModbusBlocks
		<< QUaModbusDockWidgets<T>::m_strModbusValues;
	// create access control widgets
	this->createModbusWidgetsDocks();
	// setup widgets
	this->setupModbusTreeWidget();
}

template<class T>
inline void QUaModbusDockWidgets<T>::updateWidgetsPermissions()
{
	m_modbusTreeWidget->on_loggedUserChanged(m_thiz->loggedUser());
	this->updateClientWidgetPermissions();
	this->updateBlockWidgetPermissions();
	this->updateValueWidgetPermissions();
}

template<class T>
inline void QUaModbusDockWidgets<T>::clearWidgets()
{
	m_clientWidget->clear();
	m_blockWidget->clear();
	m_valueWidget->clear();
}

template<class T>
inline void QUaModbusDockWidgets<T>::closeConfig()
{
	// clear
	this->clearWidgets();
	// reset last widgets selected
	m_client = nullptr;
	m_block  = nullptr;
	m_valueCurr  = nullptr;
}

template<class T>
inline QDomElement QUaModbusDockWidgets<T>::toDomElement(QDomDocument & domDoc) const
{
	// add element
	QDomElement elemAcDockW = domDoc.createElement(QUaModbusDockWidgets<T>::m_strXmlName);
	// serialize each widget
	for (auto wName : QUaModbusDockWidgets<T>::m_listWidgetNames)
	{
		QDomElement elemW = domDoc.createElement(QUaAcDocking::m_strXmlWidgetName);
		// set name
		elemW.setAttribute("Name", wName);
		// set permissions if any
		QUaPermissions * perms = this->getDockManager()->widgetPermissions(wName);
		if (perms)
		{
			elemW.setAttribute("Permissions", perms->nodeId());
		}
		// append
		elemAcDockW.appendChild(elemW);
	}
	// return element
	return elemAcDockW;
}

template<class T>
inline void QUaModbusDockWidgets<T>::fromDomElement(QDomElement & domElem, QString & strError)
{
	QDomNodeList listNodesW = domElem.elementsByTagName(QUaAcDocking::m_strXmlWidgetName);
	for (int i = 0; i < listNodesW.count(); i++)
	{
		QDomElement elem = listNodesW.at(i).toElement();
		Q_ASSERT(!elem.isNull());
		Q_ASSERT(QUaModbusDockWidgets<T>::m_listWidgetNames.contains(elem.attribute("Name")));
		// not having permissions is acceptable
		if (!elem.hasAttribute("Permissions"))
		{
			continue;
		}
		// attempt to add permissions
		QString strPermissionsNodeId = elem.attribute("Permissions");
		QUaNode * node = m_thiz->accessControl()->server()->nodeById(strPermissionsNodeId);
		if (!node)
		{
			strError += tr("%1 : Unexisting node with NodeId %2.")
				.arg("Error")
				.arg(strPermissionsNodeId);
			continue;
		}
		QUaPermissions * permissions = dynamic_cast<QUaPermissions*>(node);
		if (!permissions)
		{
			strError += tr("%1 : Node with NodeId %2 is not a permissions instance.")
				.arg("Error")
				.arg(strPermissionsNodeId);
			continue;
		}
		QString strWidgetName = elem.attribute("Name");
		this->getDockManager()->setWidgetPermissions(strWidgetName, permissions);
	}
}

template<class T>
inline void QUaModbusDockWidgets<T>::createModbusWidgetsDocks()
{
	m_modbusTreeWidget = new QUaModbusClientTree(m_thiz);
	this->getDockManager()->addDockWidget(
		QUaModbusDockWidgets<T>::m_strModbusTree,
		QAd::CenterDockWidgetArea,
		m_modbusTreeWidget
	);

	m_clientWidget = new QUaModbusClientWidget(m_thiz);
	auto clientArea = 
	this->getDockManager()->addDockWidget(
		QUaModbusDockWidgets<T>::m_strModbusClients,
		QAd::RightDockWidgetArea,
		m_clientWidget
	);
	m_clientWidget->setEnabled(false);
	m_clientWidget->setupPermissionsModel(m_thiz->getPermsComboModel());

	m_blockWidget = new QUaModbusDataBlockWidget(m_thiz);
	auto blockArea = 
	this->getDockManager()->addDockWidget(
		QUaModbusDockWidgets<T>::m_strModbusBlocks,
		QAd::BottomDockWidgetArea,
		m_blockWidget,
		nullptr,
		nullptr,
		clientArea
	);
	m_blockWidget->setEnabled(false);
	m_blockWidget->setupPermissionsModel(m_thiz->getPermsComboModel());

	m_valueWidget = new QUaModbusValueWidget(m_thiz);
	this->getDockManager()->addDockWidget(
		QUaModbusDockWidgets<T>::m_strModbusValues,
		QAd::BottomDockWidgetArea,
		m_valueWidget,
		nullptr,
		nullptr,
		blockArea
	);
	m_valueWidget->setEnabled(false);
	m_valueWidget->setupPermissionsModel(m_thiz->getPermsComboModel());

	// TODO : create default layouts or not?
	//this->getDockManager()->saveLayout(QUaModbusDockWidgets<T>::m_strDefault);
}

template<class T>
inline void QUaModbusDockWidgets<T>::setupModbusTreeWidget()
{
	Q_CHECK_PTR(m_modbusTreeWidget);
	// get modbus
	QUaModbusClientList * mod = m_thiz->modbusClientList();
	// set client list
	m_modbusTreeWidget->setClientList(mod);
	// bind selected
	QObject::connect(m_modbusTreeWidget, &QUaModbusClientTree::nodeSelectionChanged, this,
	[this](QUaNode * nodePrev, QModbusSelectType typePrev, QUaNode * nodeCurr, QModbusSelectType typeCurr) 
	{
		Q_UNUSED(nodePrev);
		Q_UNUSED(typePrev);
		// early exit
		if (m_thiz->isDeleting() || !nodeCurr)
		{
			return;
		}
		// set up widgets for current selection
		switch (typeCurr)
		{
		case QModbusSelectType::QUaModbusClient:
			{
				auto client = dynamic_cast<QUaModbusClient*>(nodeCurr);
				this->bindClientWidget(client);
				// clear and disable block and value widgets
				m_blockWidget->clear();
				m_blockWidget->setEnabled(false);
				m_valueWidget->clear();
				m_valueWidget->setEnabled(false);
			}
			break;
		case QModbusSelectType::QUaModbusDataBlock:
			{
				auto block = dynamic_cast<QUaModbusDataBlock*>(nodeCurr);
				this->bindBlockWidget(block);
				auto client = block->client();
				this->bindClientWidget(client);
				// clear and disable value widget
				m_valueWidget->clear();
				m_valueWidget->setEnabled(false);
			}
			break;
		case QModbusSelectType::QUaModbusValue:
			{
				auto value = dynamic_cast<QUaModbusValue*>(nodeCurr);
				this->bindValueWidget(value);
				auto block = value->block();
				this->bindBlockWidget(block);
				auto client = block->client();
				this->bindClientWidget(client);
			}
			break;
		default:
			break;
		}
	});
}

template<class T>
inline void QUaModbusDockWidgets<T>::bindClientWidget(QUaModbusClient * client)
{
	m_client = client;
	// bind widget
	m_clientWidget->setEnabled(true);
	m_clientWidget->bindClient(client);
	// permissions
	this->updateClientWidgetPermissions();
}

template<class T>
inline void QUaModbusDockWidgets<T>::bindBlockWidget(QUaModbusDataBlock * block)
{
	m_block = block;
	// bind widget
	m_blockWidget->setEnabled(true);
	m_blockWidget->bindBlock(block);
	// permissions
	this->updateBlockWidgetPermissions();
}

template<class T>
inline void QUaModbusDockWidgets<T>::bindValueWidget(QUaModbusValue * value)
{
	m_valueCurr = value;
	// bind widget
	m_valueWidget->setEnabled(true);
	m_valueWidget->bindValue(value);
	// permissions
	this->updateValueWidgetPermissions();
}

template<class T>
inline void QUaModbusDockWidgets<T>::updateClientWidgetPermissions()
{
	// if no user or client then cannot write
	auto user = m_thiz->loggedUser();
	if (!user || !m_client)
	{
		m_clientWidget->setCanWrite(false);
		return;
	}
	// client list perms
	auto permsClientList    = m_client->list()->permissionsObject();
	auto canWriteClientList = !permsClientList ? true : permsClientList->canUserWrite(user);
	m_clientWidget->setCanWriteClientList(canWriteClientList);
	// client perms
	auto perms    = m_client->permissionsObject();
	auto canWrite = !perms ? true : perms->canUserWrite(user);
	m_clientWidget->setCanWrite(canWrite);
	// block list perms
	auto permsBlockList    = m_client->dataBlocks()->permissionsObject();
	auto canWriteBlockList = !permsBlockList ? true : permsBlockList->canUserWrite(user);
	m_clientWidget->setCanWriteBlockList(canWriteBlockList);
	// NOTE : can read implemented in select tree filter
}

template<class T>
inline void QUaModbusDockWidgets<T>::updateBlockWidgetPermissions()
{
	// if no user or block then cannot write
	auto user = m_thiz->loggedUser();
	if (!user || !m_block)
	{
		m_blockWidget->setCanWrite(false);
		return;
	}
	// block list perms
	auto permsBlockList = m_block->list()->permissionsObject();
	auto canWriteBlockList = !permsBlockList ? true : permsBlockList->canUserWrite(user);
	m_blockWidget->setCanWriteBlockList(canWriteBlockList);
	// block perms
	auto perms = m_block->permissionsObject();
	auto canWrite = !perms ? true : perms->canUserWrite(user);
	m_blockWidget->setCanWrite(canWrite);
	// value list perms
	auto permsValueList = m_block->values()->permissionsObject();
	auto canWriteValueList = !permsValueList ? true : permsValueList->canUserWrite(user);
	m_blockWidget->setCanWriteValueList(canWriteValueList);
	// NOTE : can read implemented in select tree filter
}

template<class T>
inline void QUaModbusDockWidgets<T>::updateValueWidgetPermissions()
{
	// if no user or client then cannot write
	auto user = m_thiz->loggedUser();
	if (!user || !m_valueCurr)
	{
		m_valueWidget->setCanWrite(false);
		return;
	}
	// value list perms
	auto permsValueList = m_valueCurr->list()->permissionsObject();
	auto canWriteValueList = !permsValueList ? true : permsValueList->canUserWrite(user);
	m_valueWidget->setCanWriteValueList(canWriteValueList);
	// value perms
	auto perms = m_valueCurr->permissionsObject();
	auto canWrite = !perms ? true : perms->canUserWrite(user);
	m_valueWidget->setCanWrite(canWrite);
	// NOTE : can read implemented in select tree filter
}

template<class T>
inline QUaAcDocking * QUaModbusDockWidgets<T>::getDockManager() const
{
	return m_thiz->getDockManager();
}

#endif // QUAMODBUSDOCKWIDGETS_H
