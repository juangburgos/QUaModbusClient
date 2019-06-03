#ifndef QUAMODBUSCLIENTWIDGET_H
#define QUAMODBUSCLIENTWIDGET_H

#include <QWidget>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QUaNode>

namespace Ui {
class QUaModbusClientWidget;
}

class QUaModbusClient;
class QUaModbusClientList;
class QUaModbusClientDialog;
class QUaModbusDataBlock;

class QUaModbusClientWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QUaModbusClientWidget(QWidget *parent = nullptr);
    ~QUaModbusClientWidget();

	QUaModbusClientList * clientList() const;
	void setClientList(QUaModbusClientList * listClients);

	// tree headers
	enum class Headers
	{
		Objects = 0, 
		Status  = 1,
		Actions = 2,
		Invalid = 3
	};
	Q_ENUM(Headers)

	enum class SelectType
	{
		QUaModbusClient    = 0,
		QUaModbusDataBlock = 1,
		QUaModbusValue     = 2,
		Invalid = 3
	};
	Q_ENUM(SelectType)
	typedef QUaModbusClientWidget::SelectType QModbusSelectType;

signals:
	void nodeSelectionChanged(QUaNode * nodePrev, QModbusSelectType typePrev, 
		                      QUaNode * nodeCurr, QModbusSelectType typeCurr);

private slots:
    void on_pushButtonAddClient_clicked();

private:
    Ui::QUaModbusClientWidget *ui;
	QUaModbusClientList * m_listClients;
	QStandardItemModel    m_modelClients;
	QSortFilterProxyModel m_proxyClients;

	void showNewClientDialog(QUaModbusClientDialog &dialog);
	QStandardItem *  handleClientAdded  (QUaModbusClient * client);
	void showNewBlockDialog (QUaModbusClient * client, QUaModbusClientDialog &dialog);
	QStandardItem *  handleBlockAdded   (QUaModbusClient * client, QStandardItem * parent, const QString &strBlockId);
	void showNewValueDialog (QUaModbusDataBlock * block, QUaModbusClientDialog &dialog);
	QStandardItem *  handleValueAdded   (QUaModbusDataBlock * block, QStandardItem * parent, const QString &strValueId);

	static int SelectTypeRole;
	static int PointerRole;
};

typedef QUaModbusClientWidget::SelectType QModbusSelectType;

#endif // QUAMODBUSCLIENTWIDGET_H
