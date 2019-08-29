#include "quamodbusaccesscontrol.h"
#include "ui_quamodbusaccesscontrol.h"

#include <QLayout>
#include <QMessageAuthenticationCode>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>
#include <QInputDialog>
#include <QDockWidget>

#include <QUaAcCommonDialog>
#include <QUaDockWidgetPerms>
#include <QAdDockLayoutBar>

QString QUaModbusAccessControl::m_strAppName   = QObject::tr("Modbus Gateway");
QString QUaModbusAccessControl::m_strUntitiled = QObject::tr("Untitled");
QString QUaModbusAccessControl::m_strDefault   = QObject::tr("Default");

QString QUaModbusAccessControl::m_strHelpMenu = QObject::tr("HelpMenu");
QString QUaModbusAccessControl::m_strTopDock  = QObject::tr("TopDock");

QUaModbusAccessControl::QUaModbusAccessControl(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::QUaModbusAccessControl)
{
    ui->setupUi(this);
	QApplication::setApplicationName(QUaModbusAccessControl::m_strAppName);
	// initial values
	m_strTitle    = QString("%1 - %2");
	m_deleting    = false;
	m_strSecret   = "my_secret";
	m_loggedUser  = nullptr;
	this->setWindowTitle(m_strTitle.arg(QUaModbusAccessControl::m_strUntitiled).arg(QUaModbusAccessControl::m_strAppName));
	// setup opc ua information model and server
	this->setupInfoModel();
	// set permissions model for permissions combo
	this->setupPermsModel();
	// create docking manager NOTE : initially empty set by this->logout();
	m_dockManager = new QUaAcDocking(this, &m_proxyPerms);
	// create and setup access control and modbus widgets in dock
	m_acWidgets  = new QUaAcDockWidgets    <QUaModbusAccessControl>(this);
	m_modWidgets = new QUaModbusDockWidgets<QUaModbusAccessControl>(this);
	// setup native docks
	this->setupNativeDocks();
	// setup menu bar
	this->setupMenuBar();
	// handle user changed
	QObject::connect(this, &QUaModbusAccessControl::loggedUserChanged, this         , &QUaModbusAccessControl::on_loggedUserChanged);
	QObject::connect(this, &QUaModbusAccessControl::loggedUserChanged, m_dockManager, &QUaAcDocking::on_loggedUserChanged);
	// intially logged out
	this->logout();
}

QUaModbusAccessControl::~QUaModbusAccessControl()
{
	m_deleting = true;
    delete ui;
}

QUaAcDocking * QUaModbusAccessControl::getDockManager() const
{
	return m_dockManager;
}

void QUaModbusAccessControl::on_loggedUserChanged(QUaUser * user)
{
	// get menu bar widgets
	auto logginBar = dynamic_cast<QMenuBar*>(this->menuBar()->cornerWidget());
	Q_CHECK_PTR(logginBar);
	auto menuHelp = logginBar->findChild<QMenu*>(QUaModbusAccessControl::m_strHelpMenu);
	Q_CHECK_PTR(menuHelp);
	Q_ASSERT(menuHelp->actions().count() == 1);
	QAction * actLogInOut = menuHelp->actions().at(0);
	// set edit widgets permissions
	m_acWidgets->updateWidgetsPermissions();
	m_modWidgets->updateWidgetsPermissions();
	// update ui
	if (!user)
	{
		// update menubar : logged out user
		menuHelp->setTitle(tr("Login Here"));
		actLogInOut->setText(tr("Login"));
		// clear edit widgets
		m_acWidgets->clearWidgets();
		m_modWidgets->clearWidgets();
		// set empty layout
		m_dockManager->setEmptyLayout();
		return;
	}
	// update menubar : logged in user
	menuHelp->setTitle(user->getName());
	actLogInOut->setText(tr("Logout"));

	// TODO : set user's last state or layout
	m_dockManager->setEmptyLayout();
}

void QUaModbusAccessControl::on_newConfig()
{
	// clear old config (asks for confirmation is there is a config open)
	if (!this->on_closeConfig())
	{
		return;
	}
	// setup new user widget
	auto widgetNewUser = new QUaUserWidgetEdit;
	widgetNewUser->setActionsVisible(false);
	widgetNewUser->setRoleVisible(false);
	widgetNewUser->setHashVisible(false);
	widgetNewUser->setRepeatVisible(true);
	// setup dialog
	QUaAcCommonDialog dialog(this);
	dialog.setWindowTitle(tr("Create Root User"));
	dialog.setWidget(widgetNewUser);
	// show dialog
	this->showCreateRootUserDialog(dialog);
}

void QUaModbusAccessControl::on_openConfig()
{
	// setup error dialog just in case
	QMessageBox msgBox;
	msgBox.setWindowTitle(tr("Error"));
	msgBox.setIcon(QMessageBox::Critical);
	// read from file
	QString strConfigFileName = QFileDialog::getOpenFileName(this, tr("Open File"),
		QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
		tr("XML (*.xml)"));
	// validate
	if (strConfigFileName.isEmpty())
	{
		return;
	}
	// compose key file name
	QString strKeyFileName = QString("%1/%2.key").arg(QFileInfo(strConfigFileName).absoluteDir().absolutePath()).arg(QFileInfo(strConfigFileName).baseName());
	// create files
	QFile fileConfig(strConfigFileName);
	QFile fileKey(strKeyFileName);
	// exists
	if (!fileConfig.exists())
	{
		msgBox.setText(tr("File %1 does not exist.").arg(strConfigFileName));
		msgBox.exec();
		return;
	}
	else if (!fileKey.exists())
	{
		msgBox.setText(tr("File %1 does not exist.").arg(strKeyFileName));
		msgBox.exec();
		return;
	}
	else if (fileConfig.open(QIODevice::ReadOnly) && fileKey.open(QIODevice::ReadOnly))
	{
		// load config
		auto byteContents = fileConfig.readAll();
		auto byteKey = fileKey.readAll();
		// compute key
		QByteArray keyComputed = QMessageAuthenticationCode::hash(
			byteContents,
			m_strSecret.toUtf8(),
			QCryptographicHash::Sha512
		).toHex();
		// compare computed key with loaded key
		if (byteKey == keyComputed)
		{
			// close old file
			if (!this->on_closeConfig())
			{
				return;
			}
			// try load config
			auto strError = this->setXmlConfig(byteContents);
			if (strError.contains("Error"))
			{
				msgBox.setText(strError);
				msgBox.exec();
				return;
			}
			// update title
			this->setWindowTitle(m_strTitle.arg(strConfigFileName).arg(QUaModbusAccessControl::m_strAppName));
			// login
			this->login();
		}
		else
		{
			msgBox.setText(tr("Corrupted file %1.\nContents do not match key stored in %2.").arg(strConfigFileName).arg(strKeyFileName));
			msgBox.exec();
		}
	}
	else
	{
		msgBox.setText(tr("Files %1, %2 could not be opened.").arg(strConfigFileName).arg(strKeyFileName));
		msgBox.exec();
	}
}

void QUaModbusAccessControl::on_saveConfig()
{
	// select file
	QString strConfigFileName = QFileDialog::getSaveFileName(this, tr("Save File"),
		QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
		tr("XML (*.xml *.txt)"));
	// ignore if empty
	if (strConfigFileName.isEmpty() || strConfigFileName.isNull())
	{
		return;
	}
	// compose key file name
	QString strKeyFileName = QString("%1/%2.key").arg(QFileInfo(strConfigFileName).absoluteDir().absolutePath()).arg(QFileInfo(strConfigFileName).baseName());
	// save to file
	QFile fileConfig(strConfigFileName);
	QFile fileKey(strKeyFileName);
	if (fileConfig.open(QIODevice::ReadWrite | QFile::Truncate) && fileKey.open(QIODevice::ReadWrite | QFile::Truncate))
	{
		QTextStream streamConfig(&fileConfig);
		QTextStream streamKey(&fileKey);
		// save config in file
		auto byteContents = this->xmlConfig();
		streamConfig << byteContents;
		// create key
		QByteArray key = QMessageAuthenticationCode::hash(
			byteContents,
			m_strSecret.toUtf8(),
			QCryptographicHash::Sha512
		).toHex();
		// save key in file
		streamKey << key;
		// update title
		this->setWindowTitle(m_strTitle.arg(strConfigFileName).arg(QUaModbusAccessControl::m_strAppName));
	}
	else
	{
		QMessageBox msgBox;
		msgBox.setWindowTitle("Error");
		msgBox.setIcon(QMessageBox::Critical);
		msgBox.setText(tr("Error opening files %1, %2 for write operations.").arg(strConfigFileName).arg(strKeyFileName));
		msgBox.exec();
	}
	// close files
	fileConfig.close();
	fileKey.close();
}

bool QUaModbusAccessControl::on_closeConfig()
{
	// get access control
	QUaAccessControl * ac = this->accessControl();
	auto listUsers = ac->users()->users();
	// get modbus
	QUaModbusClientList * mod = this->modbusClientList();
	// are you sure wanna close?
	if (listUsers.count() > 0)
	{
		// ask user if create new config
		auto res = QMessageBox::question(
			this,
			tr("Confirm Close Config"),
			tr("Closing current configuration will discard any unsaved changes.\nWould you like to close the current configuration?"),
			QMessageBox::StandardButton::Ok,
			QMessageBox::StandardButton::Cancel
		);
		if (res != QMessageBox::StandardButton::Ok)
		{
			return false;
		}
	}
	// close dock widgets
	m_acWidgets->closeConfig();
	m_modWidgets->closeConfig();
	// logout
	this->logout();
	// clear config
	// NOTE : need to delete ac inmediatly, ac->clear(); (deleteLater) won't do the job
	ac->clearInmediatly();
	mod->clear();
	// clear dock manager (not widgets because some are fixed)
	m_dockManager->setWidgetListPermissions(nullptr);
	m_dockManager->setLayoutListPermissions(nullptr);
	for (auto widgetName : m_dockManager->widgetNames())
	{
		m_dockManager->setWidgetPermissions(widgetName, nullptr);
		// TODO : cannot remove widget because they are fixed. Should they be fixed or recreate every time?
	}
	for (auto layoutName : m_dockManager->layoutNames())
	{
		m_dockManager->setLayoutPermissions(layoutName, nullptr);
		m_dockManager->removeLayout(layoutName);
	}
	// cleanup connections
	while (m_connsModPerms.count() > 0)
	{
		QObject::disconnect(m_connsModPerms.takeFirst());
	}
	// update title
	this->setWindowTitle(m_strTitle.arg(QUaModbusAccessControl::m_strUntitiled).arg(QUaModbusAccessControl::m_strAppName));
	// success
	return true;
}

void QUaModbusAccessControl::setupInfoModel()
{
	// setup access control information model
	QUaFolderObject * objsFolder = m_server.objectsFolder();
	auto ac = objsFolder->addChild<QUaAccessControl>();
	ac->setDisplayName("AccessControl");
	ac->setBrowseName("AccessControl");
	// setup modbus gateway information model
	auto mod = objsFolder->addChild<QUaModbusClientList>();
	mod->setDisplayName("ModbusClients");
	mod->setBrowseName("ModbusClients");

	// disable anon login
	m_server.setAnonymousLoginAllowed(false);
	// define custom user validation 
	auto listUsers = ac->users();
	m_server.setUserValidationCallback(
	[listUsers](const QString &strUserName, const QString &strPassword) {
		auto user = listUsers->user(strUserName);
		if (!user)
		{
			return false;
		}
		return user->isPasswordValid(strPassword);
	});

	// setup auto add new user permissions
	QObject::connect(ac->users(), &QUaUserList::userAdded,
	[ac](QUaUser * user) {
		// return if user already has permissions
		if (user->hasPermissionsObject())
		{
			return;
		}
		// add user-only permissions
		auto permissionsList = ac->permissions();
		// create instance
		QString strPermId = QString("onlyuser_%1").arg(user->getName());
		permissionsList->addPermissions(strPermId);
		auto permissions = permissionsList->browseChild<QUaPermissions>(strPermId);
		Q_CHECK_PTR(permissions);
		// set user can read write
		permissions->addUserCanWrite(user);
		permissions->addUserCanRead(user);
		// set user's permissions
		user->setPermissionsObject(permissions);
	});
	QObject::connect(ac->users(), &QUaUserList::userRemoved,
	[ac](QUaUser * user) {
		// remove user-only permissions
		auto permissions = user->permissionsObject();
		if (!permissions)
		{
			return;
		}
		permissions->remove();
	});
	// setup auto add new role permissions
	QObject::connect(ac->roles(), &QUaRoleList::roleAdded,
	[ac](QUaRole * role) {
		// return if role already has permissions
		if (role->hasPermissionsObject())
		{
			return;
		}
		// add role-only permissions
		auto permissionsList = ac->permissions();
		// create instance
		QString strPermId = QString("onlyrole_%1").arg(role->getName());
		permissionsList->addPermissions(strPermId);
		auto permissions = permissionsList->browseChild<QUaPermissions>(strPermId);
		Q_CHECK_PTR(permissions);
		// set role can read write
		permissions->addRoleCanWrite(role); // not used
		permissions->addRoleCanRead(role);
		// set role's permissions
		role->setPermissionsObject(permissions);
	});
	QObject::connect(ac->roles(), &QUaRoleList::roleRemoved,
	[ac](QUaRole * role) {
		// remove role-only permissions
		auto permissions = role->permissionsObject();
		if (!permissions)
		{
			return;
		}
		permissions->remove();
	});
	// setup auto add new role permissions
	QObject::connect(ac->permissions(), &QUaPermissionsList::permissionsAdded,
	[ac](QUaPermissions * perms) {
		// return if permissions already has permissions
		if (perms->hasPermissionsObject())
		{
			return;
		}
		// assign to itself
		perms->setPermissionsObject(perms);
	});

	// start server
	m_server.start();
}

void QUaModbusAccessControl::setupMenuBar()
{
	// file open, save, close
	QMenu *menuFile = this->menuBar()->addMenu(tr("File"));
	menuFile->addAction(tr("New"  ), this, &QUaModbusAccessControl::on_newConfig);
	menuFile->addAction(tr("Open" ), this, &QUaModbusAccessControl::on_openConfig);
	menuFile->addSeparator();
	menuFile->addAction(tr("Save" ), this, &QUaModbusAccessControl::on_saveConfig);
	menuFile->addSeparator();
	menuFile->addAction(tr("Close"), this, &QUaModbusAccessControl::on_closeConfig);

	// native docks show/hide
	QMenu   *menuView = this->menuBar()->addMenu(tr("View"));
	QAction *actLayoutBar = menuView->addAction(tr("Layout Bar"), this, 
	[this, actLayoutBar](bool checked) {
		QDockWidget *dockTop = this->findChild<QDockWidget*>(QUaModbusAccessControl::m_strTopDock);
		Q_CHECK_PTR(dockTop);
		if (checked)
		{
			dockTop->show();
		}
		else
		{
			dockTop->hide();
		}
	});
	actLayoutBar->setCheckable(true);
	actLayoutBar->setChecked(true);

	// user widgets
	this->menuBar()->addMenu(m_dockManager->widgetsMenu());

	// user layouts
	this->menuBar()->addMenu(m_dockManager->layoutsMenu());

	// setup top right loggin menu
	auto logginBar  = new QMenuBar(this->menuBar());
	QMenu *menuHelp = new QMenu(tr("Login"), logginBar);
	menuHelp->setObjectName(QUaModbusAccessControl::m_strHelpMenu);
	logginBar->addMenu(menuHelp);
	this->menuBar()->setCornerWidget(logginBar);
	// add logon/logout action
	QAction * actLogInOut = menuHelp->addAction(tr("Login"));
	// setup action events
	QObject::connect(actLogInOut, &QAction::triggered, this,
	[this]() {
		if (this->loggedUser())
		{
			this->logout();
		}
		else
		{
			this->login();
		}
	});
}

void QUaModbusAccessControl::setupNativeDocks()
{
	// top dock - current layout name
	QDockWidget *dockTop = new QDockWidget(this);
	dockTop->setObjectName(QUaModbusAccessControl::m_strTopDock);
	dockTop->setAllowedAreas(Qt::TopDockWidgetArea);
	dockTop->setFeatures(QDockWidget::NoDockWidgetFeatures);
	dockTop->setFloating(false);
	dockTop->setTitleBarWidget(new QWidget());
	this->addDockWidget(Qt::TopDockWidgetArea, dockTop);
	// widget
	QAdDockLayoutBar *pWidget = new QAdDockLayoutBar(this, &m_proxyPerms, m_dockManager->layoutsModel());
	dockTop->setWidget(pWidget);
	// subscribe to user change
	QObject::connect(this, &QUaModbusAccessControl::loggedUserChanged, pWidget      , &QAdDockLayoutBar::on_loggedUserChanged);
	QObject::connect(this, &QUaModbusAccessControl::loggedUserChanged, m_dockManager, &QUaAcDocking    ::on_loggedUserChanged);
	// subscribe to layouts changes
	QObject::connect(m_dockManager, &QUaAcDocking::currentLayoutChanged        , pWidget, &QAdDockLayoutBar::on_currentLayoutChanged        );
	QObject::connect(m_dockManager, &QUaAcDocking::layoutPermissionsChanged    , pWidget, &QAdDockLayoutBar::on_layoutPermissionsChanged    );
	QObject::connect(m_dockManager, &QUaAcDocking::layoutListPermissionsChanged, pWidget, &QAdDockLayoutBar::on_layoutListPermissionsChanged);
	// subscribe to bar events
	QObject::connect(pWidget, &QAdDockLayoutBar::setLayout           , m_dockManager, &QUaAcDocking::setLayout           );
	QObject::connect(pWidget, &QAdDockLayoutBar::saveCurrentLayout   , m_dockManager, &QUaAcDocking::saveCurrentLayout   );
	QObject::connect(pWidget, &QAdDockLayoutBar::saveAsCurrentLayout , m_dockManager, &QUaAcDocking::saveAsCurrentLayout );
	QObject::connect(pWidget, &QAdDockLayoutBar::removeCurrentLayout , m_dockManager, &QUaAcDocking::removeCurrentLayout );
	QObject::connect(pWidget, &QAdDockLayoutBar::setLayoutPermissions, m_dockManager, &QUaAcDocking::setLayoutPermissions);
}

QUaAccessControl * QUaModbusAccessControl::accessControl() const
{
	// get access control
	QUaFolderObject * objsFolder = m_server.objectsFolder();
	QUaAccessControl * ac = objsFolder->browseChild<QUaAccessControl>("AccessControl");
	Q_CHECK_PTR(ac);
	return ac;
}

QUaModbusClientList * QUaModbusAccessControl::modbusClientList() const
{
	// get modbus client list
	QUaFolderObject * objsFolder = m_server.objectsFolder();
	QUaModbusClientList * mod = objsFolder->browseChild<QUaModbusClientList>("ModbusClients");
	Q_CHECK_PTR(mod);
	return mod;
}

bool QUaModbusAccessControl::isDeleting() const
{
	return m_deleting;
}

QUaUser * QUaModbusAccessControl::loggedUser() const
{
	return m_loggedUser;
}

void QUaModbusAccessControl::setLoggedUser(QUaUser * user)
{
	if (m_loggedUser == user)
	{
		return;
	}
	m_loggedUser = user;
	emit this->loggedUserChanged(m_loggedUser);
}

QSortFilterProxyModel * QUaModbusAccessControl::getPermsComboModel()
{
	// TODO : use to give permissions
	return &m_proxyPerms;
}

void QUaModbusAccessControl::login()
{
	// get access control
	QUaAccessControl * ac = this->accessControl();
	auto listUsers = ac->users()->users();
	// if no users yet, create root user
	if (listUsers.count() <= 0)
	{
		// ask user if create new config
		auto res = QMessageBox::question(
			this, 
			tr("No Config Loaded"), 
			tr("There is no configuration loaded.\nWould you like to create a new one?"), 
			QMessageBox::StandardButton::Ok,
			QMessageBox::StandardButton::Cancel
		);
		if (res != QMessageBox::StandardButton::Ok)
		{
			return;
		}
		// setup new user widget
		auto widgetNewUser = new QUaUserWidgetEdit;
		widgetNewUser->setActionsVisible(false);
		widgetNewUser->setRoleVisible(false);
		widgetNewUser->setHashVisible(false);
		widgetNewUser->setRepeatVisible(true);
		// setup dialog
		QUaAcCommonDialog dialog(this);
		dialog.setWindowTitle(tr("Create Root User"));
		dialog.setWidget(widgetNewUser);
		// show dialog
		this->showCreateRootUserDialog(dialog);
		return;
	}
	// if users existing, setup widget for credentials
	auto widgetNewUser = new QUaUserWidgetEdit;
	widgetNewUser->setActionsVisible(false);
	widgetNewUser->setRoleVisible(false);
	widgetNewUser->setHashVisible(false);
	// setup dialog
	QUaAcCommonDialog dialog(this);
	dialog.setWindowTitle(tr("Login Credentials"));
	dialog.setWidget(widgetNewUser);
	// show dialog
	this->showUserCredentialsDialog(dialog);
}

void QUaModbusAccessControl::logout()
{
	// logout
	this->setLoggedUser(nullptr);
	// set empty
	m_dockManager->setEmptyLayout();
}

void QUaModbusAccessControl::showCreateRootUserDialog(QUaAcCommonDialog & dialog)
{
	int res = dialog.exec();
	if (res != QDialog::Accepted)
	{
		return;
	}
	// get access control
	QUaAccessControl * ac = this->accessControl();
	auto listUsers = ac->users();
	// get widget
	auto widgetNewUser = qobject_cast<QUaUserWidgetEdit*>(dialog.widget());
	Q_CHECK_PTR(widgetNewUser);
	// get user data
	QString strUserName = widgetNewUser->userName().trimmed();
	QString strPassword = widgetNewUser->password().trimmed();
	QString strRepeat   = widgetNewUser->repeat().trimmed();
	// check pass and repeat
	if (strPassword.compare(strRepeat, Qt::CaseSensitive) != 0)
	{
		QMessageBox::critical(this, tr("Create Root User Error"), tr("Passwords do not match."), QMessageBox::StandardButton::Ok);
		this->showCreateRootUserDialog(dialog);
		return;
	}
	// check
	QString strError = listUsers->addUser(strUserName, strPassword);
	if (strError.contains("Error"))
	{
		QMessageBox::critical(this, tr("Create Root User Error"), strError, QMessageBox::StandardButton::Ok);
		this->showCreateRootUserDialog(dialog);
		return;
	}
	// else set new root user
	QUaUser * root = listUsers->user(strUserName);
	Q_CHECK_PTR(root);
	ac->setRootUser(root);
	// get root permissions object
	auto rootPermissions = root->permissionsObject();
	if (rootPermissions)
	{
		this->setRootPermissionsToLists(root);
	}
	else
	{
		QObject::connect(root, &QUaUser::permissionsObjectChanged, this,
		[this, root](QUaPermissions * rootPermissions) {
			Q_UNUSED(rootPermissions);
			this->setRootPermissionsToLists(root);
		});
	}
	// login root user
	this->setLoggedUser(root);
}

void QUaModbusAccessControl::showUserCredentialsDialog(QUaAcCommonDialog & dialog)
{
	int res = dialog.exec();
	if (res != QDialog::Accepted)
	{
		if (!this->loggedUser())
		{
			this->logout();
		}
		return;
	}
	// get access control
	QUaAccessControl * ac = this->accessControl();
	auto listUsers = ac->users();
	// get widget
	auto widgetNewUser = qobject_cast<QUaUserWidgetEdit*>(dialog.widget());
	Q_CHECK_PTR(widgetNewUser);
	// get user data
	QString strUserName = widgetNewUser->userName();
	QString strPassword = widgetNewUser->password();
	// check user
	QUaUser * user = listUsers->user(strUserName);
	if (!user)
	{
		QMessageBox::critical(this, tr("Login Credentials Error"), tr("Username %1 does not exists.").arg(strUserName), QMessageBox::StandardButton::Ok);
		this->showUserCredentialsDialog(dialog);
		return;
	}
	// check pass
	if (!user->isPasswordValid(strPassword))
	{
		QMessageBox::critical(this, tr("Login Credentials Error"), tr("Invalid password for user %1.").arg(strUserName), QMessageBox::StandardButton::Ok);
		this->showUserCredentialsDialog(dialog);
		return;
	}
	// login
	this->setLoggedUser(user);
}

void QUaModbusAccessControl::setRootPermissionsToLists(QUaUser * root)
{
	// cleanup last call
	while (m_connsModPerms.count() > 0)
	{
		QObject::disconnect(m_connsModPerms.takeFirst());
	}
	// get root permissions
	auto rootPermissions = root->permissionsObject();
	// get access control
	QUaAccessControl * ac = this->accessControl();
	// get modbus client list
	auto mod = this->modbusClientList();
	// NOTE : only root can add/remove users, roles and permissions
	ac->setPermissionsObject(rootPermissions);
	ac->users()->setPermissionsObject(rootPermissions);
	ac->roles()->setPermissionsObject(rootPermissions);
	ac->permissions()->setPermissionsObject(rootPermissions);
	// NOTE : only root can add/remove clients, blocks and values
	mod->setPermissionsObject(rootPermissions);
	// helper lambdas
	auto handleBlockAdded = 
	[this, root](QUaModbusDataBlock * block) {
		auto rootPermissions = root->permissionsObject();
		// set permissions to block list in cliet
		auto listValues = block->values();
		listValues->setPermissionsObject(rootPermissions);
	};
	auto handleClientAdded = 
	[this, root, handleBlockAdded](QUaModbusClient * client) {
		auto rootPermissions = root->permissionsObject();
		// set permissions to block list in cliet
		auto listBlocks = client->dataBlocks();
		listBlocks->setPermissionsObject(rootPermissions);
		// handle existing blocks
		for (auto block : listBlocks->blocks())
		{
			Q_CHECK_PTR(block);
			handleBlockAdded(block);
		}
		// subscribe to block added
		// NOTE : needs to be a queued connection because we want to wait until browseName is set
		m_connsModPerms <<
		QObject::connect(listBlocks, &QUaNode::childAdded, this,
		[this, client, handleBlockAdded](QUaNode * node) {
			auto block = dynamic_cast<QUaModbusDataBlock*>(node);
			Q_CHECK_PTR(block);
			handleBlockAdded(block);
		}, Qt::QueuedConnection);
	};
	// handle existing clients
	for (auto client : mod->clients())
	{
		Q_CHECK_PTR(client);
		handleClientAdded(client);
	}
	// subscribe to client added
	// NOTE : needs to be a queued connection because we want to wait until browseName is set
	m_connsModPerms << 
	QObject::connect(mod, &QUaNode::childAdded, this,
	[handleClientAdded](QUaNode * node) {
		auto client = dynamic_cast<QUaModbusClient*>(node);
		Q_CHECK_PTR(client);
		handleClientAdded(client);
	}, Qt::QueuedConnection);
}

void QUaModbusAccessControl::setupPermsModel()
{
	// setup combo model
	m_proxyPerms.setSourceModel(&m_modelPerms);
	auto listPerms = this->accessControl()->permissions();
	// add empty/undefined
	auto parent = m_modelPerms.invisibleRootItem();
	auto row = parent->rowCount();
	auto col = 0;
	auto iInvalidParam = new QStandardItem("");
	parent->setChild(row, col, iInvalidParam);
	iInvalidParam->setData(QVariant::fromValue(nullptr), QUaDockWidgetPerms::PointerRole);
	// add all existing perms
	auto perms = listPerms->permissionsList();
	for (auto perm : perms)
	{
		row = parent->rowCount();
		auto iPerm = new QStandardItem(perm->getId());
		parent->setChild(row, col, iPerm);
		iPerm->setData(QVariant::fromValue(perm), QUaDockWidgetPerms::PointerRole);
		// subscribe to destroyed
		QObject::connect(perm, &QObject::destroyed, this,
		[this, iPerm]() {
			Q_CHECK_PTR(iPerm);
			// remove from model
			m_modelPerms.removeRows(iPerm->index().row(), 1);
		});
	}
	// subscribe to perm created
	QObject::connect(listPerms, &QUaPermissionsList::permissionsAdded, this,
	[this, col](QUaPermissions * perm) {
		auto parent = m_modelPerms.invisibleRootItem();
		auto row    = parent->rowCount();
		auto iPerm  = new QStandardItem(perm->getId());
		parent->setChild(row, col, iPerm);
		iPerm->setData(QVariant::fromValue(perm), QUaDockWidgetPerms::PointerRole);
		// subscribe to destroyed
		QObject::connect(perm, &QObject::destroyed, this,
		[this, iPerm]() {
			Q_CHECK_PTR(iPerm);
			if (m_deleting)
			{
				return;
			}
			// remove from model
			m_modelPerms.removeRows(iPerm->index().row(), 1);
		});
	});
}

QByteArray QUaModbusAccessControl::xmlConfig()
{
	// create dom doc
	QDomDocument doc;
	// set xml header
	QDomProcessingInstruction header = doc.createProcessingInstruction("xml", "version='1.0' encoding='UTF-8'");
	doc.appendChild(header);
	// convert config to xml
	auto elemAc = this->toDomElement(doc);
	doc.appendChild(elemAc);
	// get contents bytearray
	return doc.toByteArray();
}

QString QUaModbusAccessControl::setXmlConfig(const QByteArray & xmlConfig)
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
	// get list of params
	QDomElement elemApp = doc.firstChildElement(QUaModbusAccessControl::staticMetaObject.className());
	if (elemApp.isNull())
	{
		strError = tr("%1 : No Application %2 element found in XML config.\n").arg("Error").arg(QUaModbusAccessControl::staticMetaObject.className());
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

QDomElement QUaModbusAccessControl::toDomElement(QDomDocument & domDoc) const
{
	// add element
	QDomElement elemApp = domDoc.createElement(QUaModbusAccessControl::staticMetaObject.className());
	// access control
	QUaAccessControl * ac = this->accessControl();
	elemApp.appendChild(ac->toDomElement(domDoc));
	// access control widgets
	elemApp.appendChild(m_acWidgets->toDomElement(domDoc));
	// modbus
	QUaModbusClientList * mod = this->modbusClientList();
	elemApp.appendChild(mod->toDomElement(domDoc));
	// modbus widgets
	elemApp.appendChild(m_modWidgets->toDomElement(domDoc));
	// layouts
	elemApp.appendChild(m_dockManager->toDomElement(domDoc));
	// return ac element
	return elemApp;
}

void QUaModbusAccessControl::fromDomElement(QDomElement & domElem, QString & strError)
{
	// access control
	QDomElement elemAc = domElem.firstChildElement(QUaAccessControl::staticMetaObject.className());
	if (elemAc.isNull())
	{
		strError = tr("%1 : No Access Control %2 element found in XML config.\n").arg("Error").arg(QUaAccessControl::staticMetaObject.className());
		return;
	}
	// access control
	QUaAccessControl * ac = this->accessControl();
	ac->fromDomElement(elemAc, strError);
	// access control widgets
	QDomElement elemAcW = domElem.firstChildElement(QUaAcDockWidgets<QUaModbusAccessControl>::m_strXmlName);
	if (elemAcW.isNull())
	{
		strError = tr("%1 : No Access Control Widgets %2 element found in XML config.\n").arg("Error").arg(QUaAcDockWidgets<QUaModbusAccessControl>::m_strXmlName);
		return;
	}
	m_acWidgets->fromDomElement(elemAcW, strError);
	// modbus
	QDomElement elemMod = domElem.firstChildElement(QUaModbusClientList::staticMetaObject.className());
	if (elemMod.isNull())
	{
		strError = tr("%1 : No Modbus Client List %2 element found in XML config.\n").arg("Error").arg(QUaModbusClientList::staticMetaObject.className());
		return;
	}
	QUaModbusClientList * mod = this->modbusClientList();
	mod->fromDomElement(elemMod, strError);
	// modbus widgets
	QDomElement elemModW = domElem.firstChildElement(QUaModbusDockWidgets<QUaModbusAccessControl>::m_strXmlName);
	if (elemModW.isNull())
	{
		strError = tr("%1 : No Modbus Widgets %2 element found in XML config.\n").arg("Error").arg(QUaModbusDockWidgets<QUaModbusAccessControl>::m_strXmlName);
		return;
	}
	m_modWidgets->fromDomElement(elemModW, strError);
	// layouts
	QDomElement elemLayouts = domElem.firstChildElement(QUaAcDocking::m_strXmlName);
	if (elemLayouts.isNull())
	{
		strError = tr("%1 : No Docking Layouts %2 element found in XML config.\n").arg("Error").arg(QUaAcDocking::m_strXmlName);
		return;
	}
	m_dockManager->fromDomElement(ac, elemLayouts, strError);
}


