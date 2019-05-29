#include "quamodbusclientwidget.h"
#include "ui_quamodbusclientwidget.h"

#include <QUaModbusClientList>

QUaModbusClientWidget::QUaModbusClientWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QUaModbusClientWidget)
{
    ui->setupUi(this);
	m_listClients = nullptr;
	// setup params tree model
	m_modelClients.setColumnCount((int)Headers::Invalid);
	QStringList paramHeaders;
	for (int i = (int)Headers::Objects; i < (int)Headers::Invalid; i++)
	{
		paramHeaders << QString(QMetaEnum::fromType<Headers>().valueToKey(i));
	}
	m_modelClients.setHorizontalHeaderLabels(paramHeaders);
	// setup params sort filter
	m_proxyClients.setSourceModel(&m_modelClients);
	// setup params table
	ui->treeViewModbus->setModel(&m_proxyClients);
	ui->treeViewModbus->setAlternatingRowColors(true);
	//ui->treeViewModbus->horizontalHeader()->setStretchLastSection(true);
	ui->treeViewModbus->setSortingEnabled(true);
	//ui->treeViewModbus->verticalHeader()->setVisible(false);
	ui->treeViewModbus->setSelectionBehavior(QAbstractItemView::SelectRows);
}

QUaModbusClientWidget::~QUaModbusClientWidget()
{
    delete ui;
}

QUaModbusClientList * QUaModbusClientWidget::clientList() const
{
	return m_listClients;
}

void QUaModbusClientWidget::setClientList(QUaModbusClientList * listClients)
{
	// check valid arg
	Q_ASSERT(listClients);
	if (!listClients) { return; }
	// check not set before
	Q_ASSERT(!m_listClients);
	if (m_listClients) { return; }
	// set
	m_listClients = listClients;
	// TODO : load initial

	// TODO : subscribe to changes

}

void QUaModbusClientWidget::on_pushButtonAddClient_clicked()
{
	// TODO : implement QUaModbusClientWidgetEdit
	// TODO : create a new dialog placeholder same as Vr to set the widget
	//        will have to create widgets for other types that require edition
}
