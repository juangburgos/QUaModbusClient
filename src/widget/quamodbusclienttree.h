#ifndef QUAMODBUSCLIENTTREE_H
#define QUAMODBUSCLIENTTREE_H

#include <QWidget>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QUaNode>

namespace Ui {
class QUaModbusClientTree;
}

class QUaModbusClient;
class QUaModbusClientList;
class QUaModbusClientDialog;
class QUaModbusDataBlock;

class QUaModbusClientTree : public QWidget
{
    Q_OBJECT

public:
    explicit QUaModbusClientTree(QWidget *parent = nullptr);
    ~QUaModbusClientTree();

	QUaModbusClientList * clientList() const;
	void setClientList(QUaModbusClientList * listClients);

	// tree headers
	enum class Headers
	{
		Objects = 0,
		Status  = 1,
		Invalid = 2
	};
	Q_ENUM(Headers)

	enum class SelectType
	{
		QUaModbusClient    = 0,
		QUaModbusDataBlock = 1,
		QUaModbusValue     = 2,
		Invalid            = 3
	};
	Q_ENUM(SelectType)
	typedef QUaModbusClientTree::SelectType QModbusSelectType;

signals:
	void nodeSelectionChanged(QUaNode * nodePrev, QModbusSelectType typePrev,
		QUaNode * nodeCurr, QModbusSelectType typeCurr);

private slots:
	void on_pushButtonAddClient_clicked();

private:
    Ui::QUaModbusClientTree *ui;
	QUaModbusClientList * m_listClients;
	QStandardItemModel    m_modelClients;
	QSortFilterProxyModel m_proxyClients;

	void setupImportButton();
	void setupExportButton();

	void showNewClientDialog(QUaModbusClientDialog &dialog);

	QStandardItem *  handleClientAdded(QUaModbusClient    * client);
	QStandardItem *  handleBlockAdded (QUaModbusClient    * client, QStandardItem * parent, const QString &strBlockId);
	QStandardItem *  handleValueAdded (QUaModbusDataBlock * block , QStandardItem * parent, const QString &strValueId);

	void    saveContentsCsvToFile(const QString &strContents) const;
	QString loadContentsCsvFromFile();
	void    displayCsvLoadResult(const QString &strError) const;

	static int SelectTypeRole;
	static int PointerRole;
};

typedef QUaModbusClientTree::SelectType QModbusSelectType;

#endif // QUAMODBUSCLIENTTREE_H
