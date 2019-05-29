#ifndef QUAMODBUSCLIENTWIDGET_H
#define QUAMODBUSCLIENTWIDGET_H

#include <QWidget>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>

namespace Ui {
class QUaModbusClientWidget;
}

class QUaModbusClientList;

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

private slots:
    void on_pushButtonAddClient_clicked();

private:
    Ui::QUaModbusClientWidget *ui;
	QUaModbusClientList * m_listClients;
	QStandardItemModel    m_modelClients;
	QSortFilterProxyModel m_proxyClients;
};

#endif // QUAMODBUSCLIENTWIDGET_H
