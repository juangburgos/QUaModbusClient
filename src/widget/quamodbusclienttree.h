#ifndef QUAMODBUSCLIENTTREE_H
#define QUAMODBUSCLIENTTREE_H

#include <QWidget>
#include <QStandardItemModel>
#include <QItemSelectionModel>
#include <QUaModbusCommonWidgets>
#include <QUaNode>

#include <QUaTreeModel>
#include <QUaTreeView>
#include <QSortFilterProxyModel>

typedef QUaTreeModel<QUaNode*, 1> QUaModbusNodeTreeModel;
typedef QUaTreeView <QUaNode*, 1> QUaModbusNodeTreeView;

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
	Q_PROPERTY(QIcon iconExpand       READ iconExpand       WRITE setIconExpand      )
	Q_PROPERTY(QIcon iconCollapse     READ iconCollapse     WRITE setIconCollapse    )
	Q_PROPERTY(QIcon iconAdd          READ iconAdd          WRITE setIconAdd         )
	Q_PROPERTY(QIcon iconDelete       READ iconDelete       WRITE setIconDelete      )
	Q_PROPERTY(QIcon iconClear        READ iconClear        WRITE setIconClear       )
	Q_PROPERTY(QIcon iconConnect      READ iconConnect      WRITE setIconConnect     )

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

	QIcon iconExpand() const;
	void  setIconExpand(const QIcon& icon);

	QIcon iconCollapse() const;
	void  setIconCollapse(const QIcon& icon);

	QIcon iconAdd   () const;
	void  setIconAdd(const QIcon& icon);

	QIcon iconDelete() const;
	void  setIconDelete(const QIcon& icon);

	QIcon iconClear () const;
	void  setIconClear(const QIcon& icon);

	QIcon iconConnect() const;
	void  setIconConnect(const QIcon& icon);

signals:
	void nodeSelectionChanged(
		QUaNode * nodePrev, 
		QModbusSelectType typePrev, 
		QUaNode * nodeCurr, 
		QModbusSelectType typeCurr
	);
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
	QUaModbusNodeTreeModel     m_modelModbus;
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
	QIcon m_iconExpand;
	QIcon m_iconCollapse;
	QIcon m_iconAdd;
	QIcon m_iconDelete;
	QIcon m_iconClear;
	QIcon m_iconConnect;

	void setupTreeContextMenu();
	void setupImportButton();
	void setupExportButton();
	void setupFilterWidgets();
	void expandRecursivelly(const QModelIndex &index, const bool &expand);

	void showNewClientDialog(QUaModbusClientDialog &dialog);

	void    saveContentsCsvToFile(const QString &strContents, const QString &strFileName = "");
	QString loadContentsCsvFromFile();
	void    displayCsvLoadResult(const QString &strError);
	void    exportAllCsv();

	bool isFilterVisible() const;
	void setFilterVisible(const bool &isVisible);

	void showNewBlockDialog(QUaModbusClient* client, QUaModbusClientDialog& dialog);
	void showNewValueDialog(QUaModbusDataBlock* block, QUaModbusClientDialog& dialog);
};

typedef QUaModbusClientTree::SelectType QModbusSelectType;

#endif // QUAMODBUSCLIENTTREE_H
