#ifndef QUAMODBUSCLIENTWIDGETTEST_H
#define QUAMODBUSCLIENTWIDGETTEST_H

#include <QDialog>

#include <QUaServer>

class QUaModbusClientWidget;
class QUaModbusDataBlockWidget;
class QUaModbusValueWidget;

class QUaModbusClient;
class QUaModbusDataBlock;
class QUaModbusValue;

namespace Ui {
class QUaModbusClientWidgetTest;
}

class QUaModbusClientWidgetTest : public QDialog
{
    Q_OBJECT

public:
    explicit QUaModbusClientWidgetTest(QWidget *parent = 0);
    ~QUaModbusClientWidgetTest();

	// tree headers
	enum class ModbusWidgets
	{
		Client  = 0,
		Block   = 1,
		Value   = 2,
		Invalid = 3
	};
	Q_ENUM(ModbusWidgets)
	typedef QUaModbusClientWidgetTest::ModbusWidgets QModbusWidgets;

signals:
	void expandTree();

private slots:
    void on_pushButtonStart_clicked();

    void on_pushButtonImport_clicked();

    void on_pushButtonExport_clicked();

private:
    Ui::QUaModbusClientWidgetTest *ui;
	QUaServer   m_server;
	bool        m_deleting;

	QUaModbusClientWidget    * m_widgetClient;
	QUaModbusDataBlockWidget * m_widgetBlock;
	QUaModbusValueWidget	 * m_widgetValue;

	void setupModbusWidgets();

	void bindClientWidget(QUaModbusClient    * client);
	void bindBlockWidget (QUaModbusDataBlock * block );
	void bindValueWidget (QUaModbusValue     * value );
};

typedef QUaModbusClientWidgetTest::ModbusWidgets QModbusWidgets;

#endif // QUAMODBUSCLIENTWIDGETTEST_H
