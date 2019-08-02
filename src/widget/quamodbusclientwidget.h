#ifndef QUAMODBUSCLIENTWIDGET_H
#define QUAMODBUSCLIENTWIDGET_H

#include <QWidget>

class QUaModbusClient;
class QUaModbusClientDialog;

namespace Ui {
class QUaModbusClientWidget;
}

class QUaModbusClientWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QUaModbusClientWidget(QWidget *parent = nullptr);
    ~QUaModbusClientWidget();

	void bindClient(QUaModbusClient * client);

	void clear();

private:
    Ui::QUaModbusClientWidget *ui;

	QList<QMetaObject::Connection> m_connections;
	
	void bindClientWidgetEdit  (QUaModbusClient * client);
	void bindClientWidgetStatus(QUaModbusClient * client);

	void showNewBlockDialog(QUaModbusClient * client, QUaModbusClientDialog &dialog);
};

#endif // QUAMODBUSCLIENTWIDGET_H
