#include "quamodbus.h"
#include "ui_quamodbus.h"

#include <QMessageBox>
#include <QFileInfo>
#include <QDir>
#include <QFileDialog>
#include <QStandardPaths>

const QString QUaModbus::m_strAppName   = QObject::tr("QUaModbusClient");
const QString QUaModbus::m_strUntitiled = QObject::tr("Untitled");
const QString QUaModbus::m_strDefault   = QObject::tr("Default");

QUaModbus::QUaModbus(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::QUaModbus)
{
    ui->setupUi(this);
	QApplication::setApplicationName(QUaModbus::m_strAppName);
	// initial values
	this->setCentralWidget(ui->mdiArea);
	this->setWindowIcon(QIcon(":/logo/logo/logo.svg"));
	m_deleting        = false;
	m_strTitle        = QString("%1 - %2");
	m_strConfigFile   = QString();
	m_strLastPathUsed = QString();
	this->setWindowTitle(m_strTitle.arg(QUaModbus::m_strUntitiled).arg(QUaModbus::m_strAppName));
	// setup opc ua information model and server
	this->setupInfoModel();
	// setup menu bar
	this->setupMenuBar();
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
		auto strError = this->setXmlConfig(byteContents);
		if (strError.contains("Error"))
		{
			msgBox.setText(strError
				.replace("Success.\n", "")
				.replace("Success", "")
				.left(600) + "..."
			);
			msgBox.exec();
			this->on_closeConfig(true);
			return;
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
			"Build   : %2\n"
			"Copyright 2019 - juangburgos\n\n"
			"Icons made by 'prettycons' from www.flaticon.com"
		).arg(strVersion).arg(strBuild)
	);
}

void QUaModbus::setupInfoModel()
{
	QUaFolderObject* objsFolder = m_server.objectsFolder();
	// setup modbus gateway information model
	auto mod = objsFolder->addChild<QUaModbusClientList>("ns=1;s=modbus.clientlist");
	mod->setDisplayName("ModbusClients");
	mod->setBrowseName("ModbusClients");

	// TODO : remove server start and provide controls to users

	// start server
	m_server.start();
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

	// TODO

	// handle help menu events
	QObject::connect(ui->actionAbout, &QAction::triggered, this, &QUaModbus::on_about);
}

void QUaModbus::clearWidgets()
{
	// TODO
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

QString QUaModbus::setXmlConfig(const QByteArray& xmlConfig)
{
	QString strError;
	// set to dom doc
	QDomDocument doc;
	int line, col;
	doc.setContent(xmlConfig, &strError, &line, &col);
	if (!strError.isEmpty())
	{
		strError = tr("%1 : Invalid XML in Line %2 Column %3 Error %4.\n").arg("Error").arg(line).arg(col).arg(strError);
		return strError;
	}
	// get config
	QDomElement elemApp = doc.firstChildElement(QUaModbus::staticMetaObject.className());
	if (elemApp.isNull())
	{
		strError = tr("%1 : No Application %2 element found in XML config.\n").arg("Error").arg(QUaModbus::staticMetaObject.className());
		return strError;
	}
	// load config from xml
	this->fromDomElement(elemApp, strError);
	if (strError.isEmpty())
	{
		strError = "Success.\n";
	}
	return strError;
}

QDomElement QUaModbus::toDomElement(QDomDocument& domDoc) const
{
	// add element
	QDomElement elemApp = domDoc.createElement(QUaModbus::staticMetaObject.className());

	// actions
	elemApp.setAttribute("ShowTree"         , ui->actionTree->isChecked()          );
	elemApp.setAttribute("ShowClientEdit"   , ui->actionClient_Edit->isChecked()   );
	elemApp.setAttribute("ShowDataBlockEdit", ui->actionDataBlock_Edit->isChecked());
	elemApp.setAttribute("ShowValueEdit"    , ui->actionValue_Edit->isChecked()    );
	// modbus
	QUaModbusClientList* mod = this->modbusClientList();
	elemApp.appendChild(mod->toDomElement(domDoc));
	// modbus widgets

	// TODO

	// return element
	return elemApp;
}

void QUaModbus::fromDomElement(QDomElement& domElem, QString& strError)
{
	// actions
	if (domElem.hasAttribute("ShowTree"))
	{
		bool ok = false;
		bool en = static_cast<bool>(domElem.attribute("ShowTree").toInt(&ok));
		if (ok)
		{
			ui->actionTree->setChecked(en);
		}
	}
	if (domElem.hasAttribute("ShowClientEdit"))
	{
		bool ok = false;
		bool en = static_cast<bool>(domElem.attribute("ShowClientEdit").toInt(&ok));
		if (ok)
		{
			ui->actionClient_Edit->setChecked(en);
		}
	}
	if (domElem.hasAttribute("ShowValueEdit"))
	{
		bool ok = false;
		bool en = static_cast<bool>(domElem.attribute("ShowValueEdit").toInt(&ok));
		if (ok)
		{
			ui->actionDataBlock_Edit->setChecked(en);
		}
	}
	if (domElem.hasAttribute("ShowDataBlockEdit"))
	{
		bool ok = false;
		bool en = static_cast<bool>(domElem.attribute("ShowDataBlockEdit").toInt(&ok));
		if (ok)
		{
			ui->actionValue_Edit->setChecked(en);
		}
	}
	// modbus
	QDomElement elemMod = domElem.firstChildElement(QUaModbusClientList::staticMetaObject.className());
	if (elemMod.isNull())
	{
		strError = tr("%1 : No Modbus Client List %2 element found in XML config.\n").arg("Error").arg(QUaModbusClientList::staticMetaObject.className());
		return;
	}
	QUaModbusClientList* mod = this->modbusClientList();
	mod->fromDomElement(elemMod, strError);
	// modbus widgets

	// TODO
}

