#include "quamodbus.h"
#include "ui_quamodbus.h"

#include <QMessageBox>
#include <QFileInfo>
#include <QDir>
#include <QFileDialog>
#include <QStandardPaths>

#include <QUaCommonDialog>
#include <QUaLogWidget>

const QString QUaModbus::m_strAppName   = QObject::tr("QUaModbusClient");
const QString QUaModbus::m_strUntitiled = QObject::tr("Untitled");
const QString QUaModbus::m_strDefault   = QObject::tr("Default");

const QString QUaModbus::m_strModbusTree    = QObject::tr("Modbus Tree");
const QString QUaModbus::m_strModbusClients = QObject::tr("Modbus Client Edit");
const QString QUaModbus::m_strModbusBlocks  = QObject::tr("Modbus DataBlock Edit");
const QString QUaModbus::m_strModbusValues  = QObject::tr("Modbus Value Edit");

QUaModbus::QUaModbus(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::QUaModbus)
{
    ui->setupUi(this);
	QApplication::setApplicationName(QUaModbus::m_strAppName);
	// initial values
	this->setWindowIcon(QIcon(":/logo/logo/logo.svg"));
	m_deleting        = false;
	m_strTitle        = QString("%1 - %2");
	m_strConfigFile   = QString();
	m_strLastPathUsed = QString();
	this->setWindowTitle(m_strTitle.arg(QUaModbus::m_strUntitiled).arg(QUaModbus::m_strAppName));
	// setup opc ua information model and server
	this->setupInfoModel();
	// setup widgets and dock
	this->setupWidgets();
	// setup menu bar
	this->setupMenuBar();
	// setup the styles (inc. icons for widgets)
	this->setupStyle();
}

QUaModbus::~QUaModbus()
{
    delete ui;
}

QUaModbusClientList* QUaModbus::modbusClientList() const
{
	// get modbus client list
	QUaFolderObject* objsFolder = m_server.objectsFolder();
	QUaModbusClientList* mod = objsFolder->browseChild<QUaModbusClientList>("ModbusClients");
	Q_CHECK_PTR(mod);
	return mod;
}

void QUaModbus::on_newConfig()
{
	// clear old config (asks for confirmation is there is a config open)
	if (!this->on_closeConfig())
	{
		return;
	}
	// any extra actions if closing succeeds must go here
}

void QUaModbus::on_openConfig()
{
	// setup error dialog just in case
	QMessageBox msgBox;
	msgBox.setWindowTitle(tr("Error"));
	msgBox.setIcon(QMessageBox::Critical);
	// read from file
	QString strConfigFileName = QFileDialog::getOpenFileName(this, tr("Open File"),
		m_strLastPathUsed.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) : m_strLastPathUsed,
		tr("XML (*.xml)"));
	// validate
	if (strConfigFileName.isEmpty())
	{
		return;
	}
	// create files
	QFile fileConfig(strConfigFileName);
	// exists
	if (!fileConfig.exists())
	{
		msgBox.setText(tr("File %1 does not exist.").arg(strConfigFileName));
		msgBox.exec();
		return;
	}
	else if (fileConfig.open(QIODevice::ReadOnly))
	{
		// save last path used
		m_strLastPathUsed = QFileInfo(fileConfig).absoluteFilePath();
		// load config
		auto byteContents = fileConfig.readAll();
		// close old file
		if (!this->on_closeConfig())
		{
			return;
		}
		// try load config
		auto errorLogs = this->setXmlConfig(byteContents);
		if (!errorLogs.isEmpty())
		{
			// setup log widget
			auto logWidget = new QUaLogWidget;
			logWidget->setFilterVisible(false);
			logWidget->setSettingsVisible(false);
			logWidget->setClearVisible(false);
			logWidget->setColumnVisible(QUaLogWidget::Columns::Timestamp, false);
			logWidget->setColumnVisible(QUaLogWidget::Columns::Category, false);
			logWidget->setLevelColor(QUaLogLevel::Error, QBrush(QColor("#8E2F1C")));
			logWidget->setLevelColor(QUaLogLevel::Warning, QBrush(QColor("#766B0F")));
			logWidget->setLevelColor(QUaLogLevel::Info, QBrush(QColor("#265EB6")));
			bool hasError = false;
			while (errorLogs.count() > 0)
			{
				auto errorLog = errorLogs.dequeue();
				hasError = hasError || errorLog.level == QUaLogLevel::Error ? true : false;
				logWidget->addLog(errorLog);
			}
			// NOTE : dialog takes ownershit
			QUaCommonDialog dialog(this);
			dialog.setWindowTitle(tr("Config Issues"));
			dialog.setWidget(logWidget);
			dialog.clearButtons();
			dialog.addButton(tr("Close"), QDialogButtonBox::ButtonRole::AcceptRole);
			dialog.exec();
			if (hasError)
			{
				this->on_closeConfig(true);
				return;
			}
		}
		// update file name
		m_strConfigFile = strConfigFileName;
		// update title
		this->setWindowTitle(m_strTitle.arg(strConfigFileName).arg(QUaModbus::m_strAppName));
	}
	else
	{
		msgBox.setText(tr("File %1 could not be opened.").arg(strConfigFileName));
		msgBox.exec();
	}
}

void QUaModbus::on_saveConfig()
{
	if (m_strConfigFile.isEmpty())
	{
		this->on_saveAsConfig();
		return;
	}
	// save to file
	QFile fileConfig(m_strConfigFile);
	if (fileConfig.open(QIODevice::ReadWrite | QFile::Truncate))
	{
		// save last path used
		m_strLastPathUsed = QFileInfo(fileConfig).absoluteFilePath();
		// create streams
		QTextStream streamConfig(&fileConfig);
		// save config in file
		auto byteContents = this->xmlConfig();
		streamConfig << byteContents;
		// update title
		this->setWindowTitle(m_strTitle.arg(m_strConfigFile).arg(QUaModbus::m_strAppName));
	}
	else
	{
		QMessageBox msgBox;
		msgBox.setWindowTitle("Error");
		msgBox.setIcon(QMessageBox::Critical);
		msgBox.setText(tr("Error opening file %1 for write operations.").arg(m_strConfigFile));
		msgBox.exec();
		// NOTE : I don't know if something needs to be handled here
	}
	// close files
	fileConfig.close();
}

void QUaModbus::on_saveAsConfig()
{
	// select file
	QString strConfigFileName = QFileDialog::getSaveFileName(this, tr("Save File"),
		m_strLastPathUsed.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) : m_strLastPathUsed,
		tr("XML (*.xml *.txt)"));
	// ignore if empty
	if (strConfigFileName.isEmpty() || strConfigFileName.isNull())
	{
		return;
	}
	// update file name
	m_strConfigFile = strConfigFileName;
	// save
	this->on_saveConfig();
}

bool QUaModbus::on_closeConfig(const bool& force)
{
	// get modbus
	QUaModbusClientList* mod = this->modbusClientList();
	// are you sure wanna close?
	if (!force)
	{
		// ask user if create new config
		auto res = QMessageBox::question(
			this,
			tr("Confirm Close Config"),
			tr(
				"Closing current configuration will discard any unsaved changes.\n"
				"Would you like to close the current configuration?"
			),
			QMessageBox::StandardButton::Ok,
			QMessageBox::StandardButton::Cancel
		);
		if (res != QMessageBox::StandardButton::Ok)
		{
			return false;
		}
	}
	// close indeed
	this->clearWidgets();
	mod->clearInmediatly();
	// update file name
	m_strConfigFile = QString();
	// update title
	this->setWindowTitle(m_strTitle.arg(QUaModbus::m_strUntitiled).arg(QUaModbus::m_strAppName));
	// success
	return true;
}

void QUaModbus::on_exit()
{
	if (!m_strConfigFile.isEmpty())
	{
		// confirmation
		auto res = QMessageBox::question(
			this,
			tr("Confirm Close Application"),
			tr("Closing the appliction. All unsaved changes will be lost.\n"
				"Would you like to continue?"),
			QMessageBox::StandardButton::Ok,
			QMessageBox::StandardButton::Cancel
		);
		if (res != QMessageBox::StandardButton::Ok)
		{
			return;
		}
	}
	// close main window
	this->close();
}

void QUaModbus::on_about()
{
	// try to find theme file
	const QString strVerFile = "version.txt";
	QString   strVerPath;
	QFileInfo infoVerFile;
	// try application path
	strVerPath  = QString("%1/%2").arg(QCoreApplication::applicationDirPath()).arg(strVerFile);
	infoVerFile = QFileInfo(strVerPath);
	// else try current path
	if (!infoVerFile.exists())
	{
		strVerPath  = QString("%1/%2").arg(QDir::currentPath()).arg(strVerFile);
		infoVerFile = QFileInfo(strVerPath);
	}
	if (!infoVerFile.exists())
	{
		QMessageBox::critical(
			this,
			tr("Version Error"),
			tr(
				"Could not load version file.\n"
				"File %1 could not be found."
			).arg(strVerFile)
		);
		return;
	}
	// load version
	QString strVersion = tr("Unknown");
	QString strBuild   = tr("Unknown");
	QFile fileVer(strVerPath);
	if (fileVer.open(QFile::ReadOnly | QFile::Text))
	{
		QTextStream ts(&fileVer);
		QString strFileText = ts.readAll();
		QStringList strRows = strFileText.split("\n");
		// load version
		if (strRows.count() >= 1 && strRows.at(0).split("=").count() >= 2)
		{
			strVersion = strRows.at(0).split("=").at(1).trimmed();
		}
		// load build
		if (strRows.count() >= 2 && strRows.at(1).split("=").count() >= 2)
		{
			strBuild = strRows.at(1).split("=").at(1).trimmed();
		}
	}
	else
	{
		QMessageBox::critical(
			this,
			tr("Version Error"),
			tr(
				"Could not open version file %1."
			).arg(strVerFile)
		);
		return;
	}
	fileVer.close();
	// show box
	QMessageBox::about(
		this,
		tr("About %1").arg(QUaModbus::m_strAppName),
		tr(
			"Version : %1\n"
			"Commit  : %2\n"
			"Source  : https://github.com/juangburgos/QUaModbusClient\n"
            "Copyright 2020 - juangburgos\n\n"
			"Icons made by 'prettycons' from https://www.flaticon.com/\n"
			"Made with Qt https://www.qt.io/\n"
		).arg(strVersion).arg(strBuild)
	);
}

void QUaModbus::setupInfoModel()
{
	QUaFolderObject* objsFolder = m_server.objectsFolder();
	// setup modbus gateway information model
	objsFolder->addChild<QUaModbusClientList>("ModbusClients", "ns=0;s=modbus.clientlist");

	// TODO : remove server start and provide controls to users

	// start server
	m_server.start();
}

void QUaModbus::setupWidgets()
{
	// setup docks
	m_dockManager = new QAdDockManager(this);
	// create tree and put it inside a dock
	m_modbusTreeWidget = new QUaModbusClientTree(m_dockManager);
	auto pDockTree = new QAdDockWidget(QUaModbus::m_strModbusTree, m_dockManager);
	pDockTree->setWidget(m_modbusTreeWidget);
	auto pAreaTree = m_dockManager->addDockWidget(QAd::CenterDockWidgetArea, pDockTree, nullptr);
	// create client edit and put it inside a dock
	m_clientWidget = new QUaModbusClientWidget(m_dockManager);
	auto pDockClientEdit = new QAdDockWidget(QUaModbus::m_strModbusClients, m_dockManager);
	pDockClientEdit->setWidget(m_clientWidget);
	auto pAreaClientEdit = m_dockManager->addDockWidget(QAd::RightDockWidgetArea, pDockClientEdit, pAreaTree);
	// disable intially
	m_clientWidget->setEnabled(false);
	// create block edit and put it inside a dock
	m_blockWidget = new QUaModbusDataBlockWidget(m_dockManager);
	auto pDockBlockEdit = new QAdDockWidget(QUaModbus::m_strModbusBlocks, m_dockManager);
	pDockBlockEdit->setWidget(m_blockWidget);
	m_dockManager->addDockWidget(QAd::RightDockWidgetArea, pDockBlockEdit, pAreaClientEdit);
	// disable intially
	m_blockWidget->setEnabled(false);
	// create value edit and put it inside a dock
	m_valueWidget = new QUaModbusValueWidget(m_dockManager);
	auto pDockValueEdit = new QAdDockWidget(QUaModbus::m_strModbusValues, m_dockManager);
	pDockValueEdit->setWidget(m_valueWidget);
	m_dockManager->addDockWidget(QAd::BottomDockWidgetArea, pDockValueEdit, pAreaClientEdit);
	// disable intially
	m_valueWidget->setEnabled(false);

	// setup widgets
	QUaModbusClientList * mod = this->modbusClientList();
	// set client list
	m_modbusTreeWidget->setClientList(mod);
	// bind selected
	QObject::connect(m_modbusTreeWidget, &QUaModbusClientTree::nodeSelectionChanged, this,
	[this](QUaNode * nodePrev, QModbusSelectType typePrev, QUaNode * nodeCurr, QModbusSelectType typeCurr) 
	{
		Q_UNUSED(nodePrev);
		Q_UNUSED(typePrev);
		// early exit
		if (m_deleting || !nodeCurr)
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
	// open client edit widget when double clicked
	QObject::connect(m_modbusTreeWidget, &QUaModbusClientTree::clientDoubleClicked, this,
	[this](QUaModbusClient * client) {
		Q_UNUSED(client);
		this->setIsDockVisible(QUaModbus::m_strModbusClients, true);
	});
	// open client block widget when double clicked
	QObject::connect(m_modbusTreeWidget, &QUaModbusClientTree::blockDoubleClicked, this,
	[this](QUaModbusDataBlock * block) {
		Q_UNUSED(block);
		this->setIsDockVisible(QUaModbus::m_strModbusBlocks, true);
	});
	// open client edit widget when double clicked
	QObject::connect(m_modbusTreeWidget, &QUaModbusClientTree::valueDoubleClicked, this,
	[this](QUaModbusValue * value) {
		Q_UNUSED(value);
		this->setIsDockVisible(QUaModbus::m_strModbusValues, true);
	});
	// clear widgets before clearing clients
	QObject::connect(m_modbusTreeWidget, &QUaModbusClientTree::aboutToClear, this,
	[this]() {
		// clear all widgets
		this->clearWidgets();
		// disable modbus widgets
		m_clientWidget->setEnabled(false);
		m_blockWidget ->setEnabled(false);
		m_valueWidget ->setEnabled(false);
	});
	// clear widgets before clearing blocks
	QObject::connect(m_clientWidget, &QUaModbusClientWidget::aboutToClear, this,
	[this]() {
		this->clearWidgets();
		// disable modbus widgets
		m_clientWidget->setEnabled(false);
		m_blockWidget ->setEnabled(false);
	});
}

void QUaModbus::setupMenuBar()
{
	// handle file menu events
	QObject::connect(ui->actionNew   , &QAction::triggered, this, &QUaModbus::on_newConfig   );
	QObject::connect(ui->actionOpen  , &QAction::triggered, this, &QUaModbus::on_openConfig  );
	QObject::connect(ui->actionSave  , &QAction::triggered, this, &QUaModbus::on_saveConfig  );
	QObject::connect(ui->actionSaveAs, &QAction::triggered, this, &QUaModbus::on_saveAsConfig);
	QObject::connect(ui->actionClose , &QAction::triggered, this, &QUaModbus::on_closeConfig );
	QObject::connect(ui->actionExit  , &QAction::triggered, this, &QUaModbus::on_exit        );
	// handle view menu events
	ui->menuView->addAction(m_dockManager->findDockWidget(QUaModbus::m_strModbusTree   )->toggleViewAction());
	ui->menuView->addAction(m_dockManager->findDockWidget(QUaModbus::m_strModbusClients)->toggleViewAction());
	ui->menuView->addAction(m_dockManager->findDockWidget(QUaModbus::m_strModbusBlocks )->toggleViewAction());
	ui->menuView->addAction(m_dockManager->findDockWidget(QUaModbus::m_strModbusValues )->toggleViewAction());
	// handle help menu events
	QObject::connect(ui->actionAbout, &QAction::triggered, this, &QUaModbus::on_about);
}

void QUaModbus::setupStyle()
{
	// try to find theme file
	const QString strThemeFile = "style.qss";
	QFileInfo infoThemeFile;
	QString   strThemePath;
	QString   strDebugThemeDir = QString("%1/../../../../res/style/default")
		.arg(QCoreApplication::applicationDirPath());
	// try application path
	strThemePath  = QString("%1/%2").arg(strDebugThemeDir).arg(strThemeFile);
	infoThemeFile = QFileInfo(strThemePath);
	// else try current path
	if (!infoThemeFile.exists())
	{
		strThemePath  = QString("%1/%2").arg(QDir::currentPath()).arg(strThemeFile);
		infoThemeFile = QFileInfo(strThemePath);
	}
	// else use embedded
	if (!infoThemeFile.exists())
	{
		strThemePath  = QString(":/style/style/default/%1").arg(strThemeFile);
		infoThemeFile = QFileInfo(strThemePath);
	}
	Q_ASSERT(infoThemeFile.exists());
	// set stylesheet
	QFile fileTheme(strThemePath);
	if (fileTheme.open(QFile::ReadOnly | QFile::Text))
	{
		QTextStream ts(&fileTheme);
		this->setStyleSheet("");
		this->setStyleSheet(ts.readAll());
		// subscribe to changes
		m_styleWatcher.addPath(strThemePath);
		QObject::connect(&m_styleWatcher, &QFileSystemWatcher::fileChanged, this,
		[this]()
		{
			this->setupStyle();
		});
	}
	else
	{
		QMessageBox::critical(
			this,
			tr("Theme Error"),
			tr(
				"Could not apply style.\n"
				"File %1 could not be opened."
			).arg(strThemeFile)
		);
	}
	fileTheme.close();
}

void QUaModbus::bindClientWidget(QUaModbusClient* client)
{
	// bind widget
	m_clientWidget->bindClient(client);
}

void QUaModbus::bindBlockWidget(QUaModbusDataBlock* block)
{
	// bind widget
	m_blockWidget->bindBlock(block);
}

void QUaModbus::bindValueWidget(QUaModbusValue* value)
{
	// bind widget
	m_valueWidget->bindValue(value);
}

void QUaModbus::clearWidgets()
{
	m_clientWidget->clear();
	m_blockWidget ->clear();
	m_valueWidget ->clear();
}

bool QUaModbus::isDockVisible(const QString& strDockName)
{
	auto dock = m_dockManager->findDockWidget(strDockName);
	Q_ASSERT_X(dock, "QUaModbus", "Invalid dock.");
	if (!dock)
	{
		return false;
	}
	return !dock->isClosed();
}

bool QUaModbus::setIsDockVisible(const QString& strDockName, const bool& visible)
{
	auto dock = m_dockManager->findDockWidget(strDockName);
	Q_ASSERT_X(dock, "QUaModbus", "Invalid dock.");
	if (!dock)
	{
		return false;
	}
	// check if already
	if (
		(!dock->isClosed() && visible) ||
		(dock->isClosed() && !visible)
		)
	{
		return true;
	}
	dock->toggleView(visible);
	return true;
}

QByteArray QUaModbus::xmlConfig()
{
	// create dom doc
	QDomDocument doc;
	// set xml header
	QDomProcessingInstruction header = doc.createProcessingInstruction("xml", "version='1.0' encoding='UTF-8'");
	doc.appendChild(header);
	// convert config to xml
	auto elem = this->toDomElement(doc);
	doc.appendChild(elem);
	// get contents bytearray
	return doc.toByteArray();
}

QQueue<QUaLog> QUaModbus::setXmlConfig(const QByteArray& xmlConfig)
{
	QQueue<QUaLog> errorLogs;
	QString strError;
	// set to dom doc
	QDomDocument doc;
	int line, col;
	doc.setContent(xmlConfig, &strError, &line, &col);
	if (!strError.isEmpty())
	{
		errorLogs << QUaLog(
			tr("Invalid XML in Line %2 Column %3 Error %4.").arg(line).arg(col).arg(strError),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		);
		return errorLogs;
	}
	// get config
	QDomElement elemApp = doc.firstChildElement(QUaModbus::staticMetaObject.className());
	if (elemApp.isNull())
	{
		errorLogs << QUaLog(
			tr("No Application %2 element found in XML config.").arg(QUaModbus::staticMetaObject.className()),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		);
		return errorLogs;
	}
	// load config from xml
	this->fromDomElement(elemApp, errorLogs);
	return errorLogs;
}

QDomElement QUaModbus::toDomElement(QDomDocument& domDoc) const
{
	// add element
	QDomElement elemApp = domDoc.createElement(QUaModbus::staticMetaObject.className());
	// modbus
	QUaModbusClientList* mod = this->modbusClientList();
	elemApp.appendChild(mod->toDomElement(domDoc));
	// dock
	elemApp.setAttribute("DockState", QString(m_dockManager->saveState().toHex()));
	// return element
	return elemApp;
}

void QUaModbus::fromDomElement(QDomElement& domElem, QQueue<QUaLog>& errorLogs)
{
	// modbus
	QDomElement elemMod = domElem.firstChildElement(QUaModbusClientList::staticMetaObject.className());
	if (elemMod.isNull())
	{
		errorLogs << QUaLog(
			tr("No Modbus Client List %1 element found in XML config.").arg(QUaModbusClientList::staticMetaObject.className()),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		);
		return;
	}
	QUaModbusClientList* mod = this->modbusClientList();
	mod->fromDomElement(elemMod, errorLogs);
	// dock
	if (domElem.hasAttribute("DockState"))
	{
		m_dockManager->restoreState(QByteArray::fromHex(domElem.attribute("DockState").toUtf8()));
	}
}

