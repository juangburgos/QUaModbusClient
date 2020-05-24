#ifndef QUAMODBUS_H
#define QUAMODBUS_H

#include <QMainWindow>
#include <QMenuBar>
#include <QFileSystemWatcher>

#include <QUaServer>

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

#include <DockManager.h>
#include <DockWidget.h>
#include <DockAreaWidget.h>
#include <DockWidgetTab.h>

namespace QAd = ads;
typedef QAd::CDockManager    QAdDockManager;
typedef QAd::CDockWidget     QAdDockWidget;
typedef QAd::DockWidgetArea  QAdDockArea;
typedef QAd::CDockAreaWidget QAdDockWidgetArea;
typedef QAd::CDockWidgetTab  QAdDockWidgetTab;

QT_BEGIN_NAMESPACE
namespace Ui { class QUaModbus; }
QT_END_NAMESPACE

class QUaModbus : public QMainWindow
{
    Q_OBJECT

public:
    QUaModbus(QWidget *parent = nullptr);
    ~QUaModbus();

	QUaModbusClientList* modbusClientList() const;

private slots:
	void on_newConfig();
	void on_openConfig();
	void on_saveConfig();
	void on_saveAsConfig();
	bool on_closeConfig(const bool& force = false);
	void on_exit();
	void on_about();

private:
    Ui::QUaModbus *ui;

	// window members
	QUaServer m_server;
	bool      m_deleting;
	QString   m_strTitle;
	QString   m_strConfigFile;
	QString   m_strLastPathUsed;
	QFileSystemWatcher m_styleWatcher;

	// widgets
	QUaModbusClientTree      * m_modbusTreeWidget;
	QUaModbusClientWidget    * m_clientWidget    ;
	QUaModbusDataBlockWidget * m_blockWidget     ;
	QUaModbusValueWidget	 * m_valueWidget     ; 
	QAdDockManager           * m_dockManager     ;

	// constants
	const static QString m_strAppName;
	const static QString m_strUntitiled;
	const static QString m_strDefault;
	const static QString m_strModbusTree;
	const static QString m_strModbusClients;
	const static QString m_strModbusBlocks;
	const static QString m_strModbusValues;

	// methods
	void setupInfoModel();
	void setupWidgets();
	void setupMenuBar();
	void setupStyle();

	void bindClientWidget(QUaModbusClient    * client);
	void bindBlockWidget (QUaModbusDataBlock * block );
	void bindValueWidget (QUaModbusValue     * value );
	void clearWidgets();

	bool isDockVisible(const QString& strDockName);
	bool setIsDockVisible(const QString& strDockName, const bool& visible);

	// xml import / export
	QByteArray     xmlConfig();
	QQueue<QUaLog> setXmlConfig(const QByteArray& xmlConfig);
	QDomElement    toDomElement(QDomDocument& domDoc) const;
	void           fromDomElement(QDomElement& domElem, QQueue<QUaLog>& errorLogs);
};
#endif // QUAMODBUS_H
