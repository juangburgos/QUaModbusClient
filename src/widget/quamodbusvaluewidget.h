#ifndef QUAMODBUSVALUEWIDGET_H
#define QUAMODBUSVALUEWIDGET_H

#include <QWidget>

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

	// TODO : add import export CSV

private:
    Ui::QUaModbusValueWidget *ui;

	QList<QMetaObject::Connection> m_connections;

	void bindValueWidgetEdit   (QUaModbusValue * value);
	void bindValueWidgetStatus (QUaModbusValue * value);
};

#endif // QUAMODBUSVALUEWIDGET_H
