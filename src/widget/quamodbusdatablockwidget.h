#ifndef QUAMODBUSDATABLOCKWIDGET_H
#define QUAMODBUSDATABLOCKWIDGET_H

#include <QWidget>

class QUaModbusDataBlock;
class QUaModbusClientDialog;

namespace Ui {
class QUaModbusDataBlockWidget;
}

class QUaModbusDataBlockWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QUaModbusDataBlockWidget(QWidget *parent = nullptr);
    ~QUaModbusDataBlockWidget();

	void bindBlock(QUaModbusDataBlock * block);

	void clear();

private:
    Ui::QUaModbusDataBlockWidget *ui;

	QList<QMetaObject::Connection> m_connections;

	void bindBlockWidgetEdit   (QUaModbusDataBlock * block );
	void bindBlockWidgetStatus (QUaModbusDataBlock * block );

	void showNewValueDialog(QUaModbusDataBlock * block, QUaModbusClientDialog & dialog);
};

#endif // QUAMODBUSDATABLOCKWIDGET_H
