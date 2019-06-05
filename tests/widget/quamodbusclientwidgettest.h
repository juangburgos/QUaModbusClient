#ifndef QUAMODBUSCLIENTWIDGETTEST_H
#define QUAMODBUSCLIENTWIDGETTEST_H

#include <QDialog>

#include <QUaServer>

#include <QUaModbusClientWidget>

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

private slots:
    void on_pushButtonStart_clicked();

    void on_pushButtonImport_clicked();

    void on_pushButtonExport_clicked();

private:
    Ui::QUaModbusClientWidgetTest *ui;
	QWidget   * m_pWidgetEdit;
	QWidget   * m_pWidgetStatus;
	QUaServer   m_server;
	bool        m_deleting;

	void bindClientWidgetEdit  (QUaModbusClient    * client);
	void bindBlockWidgetEdit   (QUaModbusDataBlock * block );
	void bindValueWidgetEdit   (QUaModbusValue     * value );
	void bindClientWidgetStatus(QUaModbusClient    * client);
	void bindBlockWidgetStatus (QUaModbusDataBlock * block );
	void bindValueWidgetStatus (QUaModbusValue     * value );

};

#endif // QUAMODBUSCLIENTWIDGETTEST_H
