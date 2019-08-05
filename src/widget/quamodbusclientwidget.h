#ifndef QUAMODBUSCLIENTWIDGET_H
#define QUAMODBUSCLIENTWIDGET_H

#include <QWidget>

#ifdef QUA_ACCESS_CONTROL
#include <QSortFilterProxyModel>
#endif // QUA_ACCESS_CONTROL

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

#ifdef QUA_ACCESS_CONTROL
	void setupPermissionsModel(QSortFilterProxyModel * proxyPerms);
	void setCanWrite(const bool &canWrite);
	// TODO : void setCanWriteList(const bool &canWrite);
#endif // QUA_ACCESS_CONTROL

private:
    Ui::QUaModbusClientWidget *ui;

	QList<QMetaObject::Connection> m_connections;
	
	void bindClientWidgetEdit  (QUaModbusClient * client);
	void bindClientWidgetStatus(QUaModbusClient * client);

	void showNewBlockDialog(QUaModbusClient * client, QUaModbusClientDialog &dialog);

#ifdef QUA_ACCESS_CONTROL
	QSortFilterProxyModel * m_proxyPerms;
#endif // QUA_ACCESS_CONTROL

};

#endif // QUAMODBUSCLIENTWIDGET_H
