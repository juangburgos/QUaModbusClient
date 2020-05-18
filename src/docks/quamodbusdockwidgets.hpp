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

	// CSV export
	void exportAllCsv(const QString &strBaseName);

	const static QString m_strXmlName;

private:
	T * m_thiz;

	// modbus widgets
	QUaModbusClientTree      * m_modbusTreeWidget;
	QUaModbusClientWidget    * m_clientWidget;
	QUaModbusDataBlockWidget * m_blockWidget ;
	QUaModbusValueWidget	 * m_valueWidget ; 

	void createModbusWidgetsDocks();
	void setupModbusTreeWidget();

	void bindClientWidget(QUaModbusClient    * client);
	void bindBlockWidget (QUaModbusDataBlock * block );
	void bindValueWidget (QUaModbusValue     * value );

	// helpers
	QUaAcDocking * getDockManager() const;

	const static QString m_strMenuPath;
	const static QString m_strModbusTree;
	const static QString m_strModbusClients;
	const static QString m_strModbusBlocks;
	const static QString m_strModbusValues;

	static QList<QString> m_listWidgetNames;
};

template <class T>
const QString QUaModbusDockWidgets<T>::m_strXmlName = "QUaModbusDockWidgets";

template <class T>
const QString QUaModbusDockWidgets<T>::m_strMenuPath = "Modbus";

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
	// element names list to serialize
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
	m_clientWidget    ->on_loggedUserChanged(m_thiz->loggedUser());
	m_blockWidget     ->on_loggedUserChanged(m_thiz->loggedUser());
	m_valueWidget     ->on_loggedUserChanged(m_thiz->loggedUser());
}

template<class T>
inline void QUaModbusDockWidgets<T>::clearWidgets()
{
	m_clientWidget->clear();
	m_blockWidget ->clear();
	m_valueWidget ->clear();
}

template<class T>
inline void QUaModbusDockWidgets<T>::closeConfig()
{
	// clear
	this->clearWidgets();
}

template<class T>
inline QDomElement QUaModbusDockWidgets<T>::toDomElement(QDomDocument & domDoc) const
{
	// add element
	QDomElement elemAcDockW = domDoc.createElement(QUaModbusDockWidgets<T>::m_strXmlName);
	// serialize each widget
	for (auto wName : QUaModbusDockWidgets<T>::m_listWidgetNames)
	{
		QDomElement elemW = domDoc.createElement(QUaAcDocking::m_strXmlDockName);
		// set name
		elemW.setAttribute("Name", wName);
		// set permissions if any
		QUaPermissions * perms = this->getDockManager()->dockPermissions(wName);
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
	QDomNodeList listNodesW = domElem.elementsByTagName(QUaAcDocking::m_strXmlDockName);
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
		QUaPermissions * permissions = qobject_cast<QUaPermissions*>(node);
		if (!permissions)
		{
			strError += tr("%1 : Node with NodeId %2 is not a permissions instance.")
				.arg("Error")
				.arg(strPermissionsNodeId);
			continue;
		}
		QString strWidgetName = elem.attribute("Name");
		this->getDockManager()->setDockPermissions(strWidgetName, permissions);
	}
}

template<class T>
inline void QUaModbusDockWidgets<T>::exportAllCsv(const QString & strBaseName)
{
	m_modbusTreeWidget->exportAllCsv(strBaseName);
}

template<class T>
inline void QUaModbusDockWidgets<T>::createModbusWidgetsDocks()
{
	m_modbusTreeWidget = new QUaModbusClientTree(m_thiz);
	this->getDockManager()->addDock(
		QUaModbusDockWidgets<T>::m_strMenuPath + "/" + QUaModbusDockWidgets<T>::m_strModbusTree,
		QAd::CenterDockWidgetArea,
		m_modbusTreeWidget
	);
	m_modbusTreeWidget->setupPermissionsModel(m_thiz->getPermsComboModel());

	m_clientWidget = new QUaModbusClientWidget(m_thiz); 
	this->getDockManager()->addDock(
		QUaModbusDockWidgets<T>::m_strMenuPath + "/" + QUaModbusDockWidgets<T>::m_strModbusClients,
		QAd::RightDockWidgetArea,
		m_clientWidget
	);
	m_clientWidget->setEnabled(false);
	m_clientWidget->setupPermissionsModel(m_thiz->getPermsComboModel());

	m_blockWidget = new QUaModbusDataBlockWidget(m_thiz);
	this->getDockManager()->addDock(
		QUaModbusDockWidgets<T>::m_strMenuPath + "/" + QUaModbusDockWidgets<T>::m_strModbusBlocks,
		QAd::BottomDockWidgetArea,
		m_blockWidget
	);
	m_blockWidget->setEnabled(false);
	m_blockWidget->setupPermissionsModel(m_thiz->getPermsComboModel());

	m_valueWidget = new QUaModbusValueWidget(m_thiz);
	this->getDockManager()->addDock(
		QUaModbusDockWidgets<T>::m_strMenuPath + "/" + QUaModbusDockWidgets<T>::m_strModbusValues,
		QAd::BottomDockWidgetArea,
		m_valueWidget
	);
	m_valueWidget->setEnabled(false);
	m_valueWidget->setupPermissionsModel(m_thiz->getPermsComboModel());
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
	QObject::connect(m_modbusTreeWidget, &QUaModbusClientTree::nodeSelectionChanged, m_thiz,
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
				auto client = qobject_cast<QUaModbusClient*>(nodeCurr);
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
				auto block = qobject_cast<QUaModbusDataBlock*>(nodeCurr);
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
				auto value = qobject_cast<QUaModbusValue*>(nodeCurr);
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
	// open client edit widget when double clicked
	QObject::connect(m_modbusTreeWidget, &QUaModbusClientTree::clientDoubleClicked, this,
	[this](QUaModbusClient * client) {
		Q_UNUSED(client);
		this->getDockManager()->setIsDockVisible(QUaModbusDockWidgets<T>::m_strModbusClients, true);
	});
	// open client block widget when double clicked
	QObject::connect(m_modbusTreeWidget, &QUaModbusClientTree::blockDoubleClicked, this,
	[this](QUaModbusDataBlock * block) {
		Q_UNUSED(block);
		this->getDockManager()->setIsDockVisible(QUaModbusDockWidgets<T>::m_strModbusBlocks, true);
	});
	// open client edit widget when double clicked
	QObject::connect(m_modbusTreeWidget, &QUaModbusClientTree::valueDoubleClicked, this,
	[this](QUaModbusValue * value) {
		Q_UNUSED(value);
		this->getDockManager()->setIsDockVisible(QUaModbusDockWidgets<T>::m_strModbusValues, true);
	});
	// clear widgets before clearing clients
	QObject::connect(m_modbusTreeWidget, &QUaModbusClientTree::aboutToClear, m_thiz,
	[this]() {
		// clear all widgets
		m_thiz->clearAllWidgets();
		// disable modbus widgets
		m_clientWidget->setEnabled(false);
		m_blockWidget ->setEnabled(false);
		m_valueWidget ->setEnabled(false);
	});
	// clear widgets before clearing blocks
	QObject::connect(m_clientWidget, &QUaModbusClientWidget::aboutToClear, m_thiz,
	[this]() {
		m_thiz->clearAllWidgets();
		// disable modbus widgets
		m_clientWidget->setEnabled(false);
		m_blockWidget ->setEnabled(false);
	});
}

template<class T>
inline void QUaModbusDockWidgets<T>::bindClientWidget(QUaModbusClient * client)
{
	// bind widget
	m_clientWidget->bindClient(client);
	// permissions
	m_clientWidget->on_loggedUserChanged(m_thiz->loggedUser());
}

template<class T>
inline void QUaModbusDockWidgets<T>::bindBlockWidget(QUaModbusDataBlock * block)
{
	// bind widget
	m_blockWidget->bindBlock(block);
	// permissions
	m_blockWidget->on_loggedUserChanged(m_thiz->loggedUser());
}

template<class T>
inline void QUaModbusDockWidgets<T>::bindValueWidget(QUaModbusValue * value)
{
	// bind widget
	m_valueWidget->bindValue(value);
	// permissions
	m_valueWidget->on_loggedUserChanged(m_thiz->loggedUser());
}

template<class T>
inline QUaAcDocking * QUaModbusDockWidgets<T>::getDockManager() const
{
	return m_thiz->getDockManager();
}

#endif // QUAMODBUSDOCKWIDGETS_H
