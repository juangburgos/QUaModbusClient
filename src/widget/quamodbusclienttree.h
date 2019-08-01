#ifndef QUAMODBUSCLIENTTREE_H
#define QUAMODBUSCLIENTTREE_H

#include <QWidget>
#include <QStandardItemModel>
#include <QUaModbusCommonWidgets>
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

	void setExpanded(const bool &expanded = true);

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

	// filter combo options
	enum class ComboOpts
	{
		Clients = 0,
		Blocks  = 1,
		Values  = 2
	};
	Q_ENUM(ComboOpts)

signals:
	void nodeSelectionChanged(QUaNode * nodePrev, QModbusSelectType typePrev,
		QUaNode * nodeCurr, QModbusSelectType typeCurr);

private slots:
	void on_pushButtonAddClient_clicked();

    void on_checkBoxFilter_toggled(bool checked);

    void on_comboBoxFilterType_currentIndexChanged(int index);

    void on_lineEditFilterText_textChanged(const QString &arg1);

private:
    Ui::QUaModbusClientTree *ui;
	QUaModbusClientList * m_listClients;
	QStandardItemModel         m_modelModbus;
	QUaModbusLambdaFilterProxy m_proxyModbus;

	void setupTreeContextMenu();
	void setupImportButton();
	void setupExportButton();
	void setupFilterWidgets();
	void expandRecursivelly(const QModelIndex &index, const bool &expand);

	void showNewClientDialog(QUaModbusClientDialog &dialog);

	QStandardItem * handleClientAdded(QUaModbusClient    * client);
	QStandardItem * handleBlockAdded (QUaModbusClient    * client, QStandardItem * parent, const QString &strBlockId);
	QStandardItem * handleValueAdded (QUaModbusDataBlock * block , QStandardItem * parent, const QString &strValueId);

	void    saveContentsCsvToFile(const QString &strContents) const;
	QString loadContentsCsvFromFile();
	void    displayCsvLoadResult(const QString &strError) const;

	bool isFilterVisible() const;
	void setFilterVisible(const bool &isVisible);

	static int SelectTypeRole;
	static int PointerRole;
};

typedef QUaModbusClientTree::SelectType QModbusSelectType;

#endif // QUAMODBUSCLIENTTREE_H
