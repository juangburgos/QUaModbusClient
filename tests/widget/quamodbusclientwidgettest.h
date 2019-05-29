#ifndef QUAMODBUSCLIENTWIDGETTEST_H
#define QUAMODBUSCLIENTWIDGETTEST_H

#include <QDialog>

#include <QUaServer>

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

	QUaServer m_server;
};

#endif // QUAMODBUSCLIENTWIDGETTEST_H
