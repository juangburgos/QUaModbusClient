#ifndef QUAMODBUSACCESSCONTROL_H
#define QUAMODBUSACCESSCONTROL_H

#include <QMainWindow>
#include <QMenuBar>

#include <QUaServer>
#include <QUaAcDockWidgets>

#include <QUaModbusDockWidgets>

#include <QUaModbusClientList>

namespace Ui {
class QUaModbusAccessControl;
}

class QUaModbusAccessControl : public QMainWindow
{
    Q_OBJECT

public:
    explicit QUaModbusAccessControl(QWidget *parent = nullptr);
    ~QUaModbusAccessControl();

	// NOTE : all public methods (including signals) are T requirements

	QUaAcDocking * getDockManager() const;

	QUaAccessControl * accessControl() const;

	QUaModbusClientList * modbusClientList() const;

	bool isDeleting() const;

	QUaUser * loggedUser() const;
	void      setLoggedUser(QUaUser * user);

	QSortFilterProxyModel * getPermsComboModel();

signals:
	void loggedUserChanged(QUaUser * user);

private slots:
	void on_loggedUserChanged(QUaUser * user);

	void on_newConfig();
	void on_openConfig();
	void on_saveConfig();
	bool on_closeConfig();

private:
    Ui::QUaModbusAccessControl *ui;

	// window members
	QUaServer        m_server;
	bool             m_deleting;
	QString          m_strSecret;
	QString          m_strTitle;
	QUaUser        * m_loggedUser;
	QUaAcDocking   * m_dockManager;

	// NOTE : template arg must fullfill T requirements
	QUaAcDockWidgets    <QUaModbusAccessControl> * m_acWidgets;
	QUaModbusDockWidgets<QUaModbusAccessControl> * m_modWidgets;

	void setupInfoModel();
	void setupNativeDocks();
	void setupMenuBar();

	void login();
	void logout();
	void showCreateRootUserDialog(QUaAcCommonDialog &dialog);
	void showUserCredentialsDialog(QUaAcCommonDialog &dialog);

	QList<QMetaObject::Connection> m_connsModPerms;
	void setRootPermissionsToLists(QUaUser * root);

	// permissions model for combobox
	QStandardItemModel    m_modelPerms;
	QSortFilterProxyModel m_proxyPerms;
	void setupPermsModel();

	// XML import / export
	QByteArray xmlConfig();
	QString    setXmlConfig(const QByteArray &xmlConfig);

	QDomElement toDomElement(QDomDocument & domDoc) const;
	void        fromDomElement(QDomElement  & domElem, QString &strError);

	static QString m_strAppName;
	static QString m_strUntitiled;
	static QString m_strDefault;

	// to find children
	static QString m_strHelpMenu;
	static QString m_strTopDock;
};

#endif // QUAMODBUSACCESSCONTROL_H
