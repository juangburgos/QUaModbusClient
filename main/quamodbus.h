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

	// constants
	const static QString m_strAppName;
	const static QString m_strUntitiled;
	const static QString m_strDefault;

	// methods
	void setupInfoModel();
	void setupMenuBar();

	void clearWidgets();

	// xml import / export
	QByteArray  xmlConfig();
	QString     setXmlConfig(const QByteArray& xmlConfig);
	QDomElement toDomElement(QDomDocument& domDoc) const;
	void        fromDomElement(QDomElement& domElem, QString& strError);
};
#endif // QUAMODBUS_H
