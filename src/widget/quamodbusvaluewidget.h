#ifndef QUAMODBUSVALUEWIDGET_H
#define QUAMODBUSVALUEWIDGET_H

#include <QWidget>

#ifdef QUA_ACCESS_CONTROL
#include <QSortFilterProxyModel>
class QUaUser;
#endif // QUA_ACCESS_CONTROL

class QUaModbusValue;
class QUaModbusClientDialog;

namespace Ui {
class QUaModbusValueWidget;
}

class QUaModbusValueWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QUaModbusValueWidget(QWidget *parent = nullptr);
    ~QUaModbusValueWidget();

	void bindValue(QUaModbusValue * value);

	void clear();

#ifdef QUA_ACCESS_CONTROL
	void setupPermissionsModel(QSortFilterProxyModel * proxyPerms);
signals:
	// NOTE : internal signal
	void loggedUserChanged();
public slots:
	void on_loggedUserChanged(QUaUser * user);
#endif // QUA_ACCESS_CONTROL

private:
    Ui::QUaModbusValueWidget *ui;

	QList<QMetaObject::Connection> m_connections;

	void bindValueWidgetEdit   (QUaModbusValue * value);
	void bindValueWidgetStatus (QUaModbusValue * value);

#ifdef QUA_ACCESS_CONTROL
	QUaUser * m_loggedUser;
	QSortFilterProxyModel * m_proxyPerms;
#endif // QUA_ACCESS_CONTROL

};

#endif // QUAMODBUSVALUEWIDGET_H
