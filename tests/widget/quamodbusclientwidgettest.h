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

private:
    Ui::QUaModbusClientWidgetTest *ui;
	QWidget         * m_pWidgetEdit;
	QModbusSelectType m_typeModbusCurr;
	QUaServer         m_server;

	void bindClientWidgetEdit(QUaModbusClient    * client);
	void bindBlockWidgetEdit (QUaModbusDataBlock * block );
	void bindValueWidgetEdit (QUaModbusValue     * value );

};

#endif // QUAMODBUSCLIENTWIDGETTEST_H
