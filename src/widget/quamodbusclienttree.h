#ifndef QUAMODBUSCLIENTTREE_H
#define QUAMODBUSCLIENTTREE_H

#include <QWidget>
#include <QStandardItemModel>
#include <QItemSelectionModel>
#include <QUaModbusCommonWidgets>
#include <QUaNode>

namespace Ui {
class QUaModbusClientTree;
}

class QUaModbusClient;
class QUaModbusClientList;
class QUaModbusClientDialog;
class QUaModbusDataBlock;
class QUaModbusValue;

#ifdef QUA_ACCESS_CONTROL
class QUaUser;
#endif // QUA_ACCESS_CONTROL

class QUaModbusClientTree : public QWidget
{
    Q_OBJECT

	// expose to style
	Q_PROPERTY(QIcon iconClientTcp    READ iconClientTcp    WRITE setIconClientTcp   )
	Q_PROPERTY(QIcon iconClientSerial READ iconClientSerial WRITE setIconClientSerial)
	Q_PROPERTY(QIcon iconBlock        READ iconBlock        WRITE setIconBlock       )
	Q_PROPERTY(QIcon iconValue        READ iconValue        WRITE setIconValue       )

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

	// get selection model to block it if necessary
	QItemSelectionModel * selectionModel() const;

	// will append _clients, _blocks, _values to baseName
	// NOTE : baseName must contain path already e.g.
	// PATH/project.xml ->  PATH/project_clients.xml, PATH/project_blocks.xml, PATH/project_values.xml
	void exportAllCsv(const QString &strBaseName);

#ifdef QUA_ACCESS_CONTROL
	void setupPermissionsModel(QSortFilterProxyModel * proxyPerms);
#endif // QUA_ACCESS_CONTROL

	// stylesheet

	QIcon iconClientTcp() const;
	void  setIconClientTcp(const QIcon &icon);

	QIcon iconClientSerial() const;
	void  setIconClientSerial(const QIcon &icon);

	QIcon iconBlock() const;
	void  setIconBlock(const QIcon &icon);

	QIcon iconValue() const;
	void  setIconValue(const QIcon &icon);

signals:
	void nodeSelectionChanged(QUaNode * nodePrev, QModbusSelectType typePrev, QUaNode * nodeCurr, QModbusSelectType typeCurr);
	void clientDoubleClicked (QUaModbusClient    * client);
	void blockDoubleClicked  (QUaModbusDataBlock * block );
	void valueDoubleClicked  (QUaModbusValue     * value );
	void aboutToClear();

public slots:
#ifdef QUA_ACCESS_CONTROL
	void on_loggedUserChanged(QUaUser * user);
#endif // QUA_ACCESS_CONTROL

private slots:
	void on_pushButtonAddClient_clicked();

    void on_pushButtonClear_clicked();

    void on_checkBoxFilter_toggled(bool checked);

    void on_comboBoxFilterType_currentIndexChanged(int index);

    void on_lineEditFilterText_textChanged(const QString &arg1);

private:
    Ui::QUaModbusClientTree  * ui;
	QUaModbusClientList      * m_listClients;
	QStandardItemModel         m_modelModbus;
	QUaModbusLambdaFilterProxy m_proxyModbus;
#ifdef QUA_ACCESS_CONTROL
	QUaUser * m_loggedUser;
	QSortFilterProxyModel * m_proxyPerms;
#endif // QUA_ACCESS_CONTROL
	QString m_strLastPathUsed;

	QIcon m_iconClientTcp;
	QIcon m_iconClientSerial;
	QIcon m_iconBlock;
	QIcon m_iconValue;

	void setupTreeContextMenu();
	void setupImportButton();
	void setupExportButton();
	void setupFilterWidgets();
	void expandRecursivelly(const QModelIndex &index, const bool &expand);

	typedef std::function<bool(QStandardItem*)> QUaModbusFuncSetIcon;
	void updateIconRecursive(QStandardItem * parent, const quint32 &depth, const QIcon &icon, const QUaModbusFuncSetIcon &func);

	void showNewClientDialog(QUaModbusClientDialog &dialog);

	QStandardItem * handleClientAdded(QUaModbusClient    * client);
	QStandardItem * handleBlockAdded (QUaModbusClient    * client, QStandardItem * parent, const QString &strBlockId);
	QStandardItem * handleValueAdded (QUaModbusDataBlock * block , QStandardItem * parent, const QString &strValueId);

	void    saveContentsCsvToFile(const QString &strContents, const QString &strFileName = "");
	QString loadContentsCsvFromFile();
	void    displayCsvLoadResult(const QString &strError);
	void    exportAllCsv();

	bool isFilterVisible() const;
	void setFilterVisible(const bool &isVisible);

	static int SelectTypeRole;
	static int PointerRole;
};

typedef QUaModbusClientTree::SelectType QModbusSelectType;

#endif // QUAMODBUSCLIENTTREE_H