#include "quamodbusdatablockwidgetstatus.h"
#include "ui_quamodbusdatablockwidgetstatus.h"

QUaModbusDataBlockWidgetStatus::QUaModbusDataBlockWidgetStatus(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QUaModbusDataBlockWidgetStatus)
{
    ui->setupUi(this);
	// setup params table
	ui->tableViewData->setModel(&m_modelValues);
	ui->tableViewData->setAlternatingRowColors(true);
	ui->tableViewData->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->tableViewData->horizontalHeader()->setStretchLastSection(true);
	// setup model
	m_modelValues.setColumnCount(1);
	m_modelValues.setHorizontalHeaderLabels(QStringList() << tr("Decimal Value"));

	// TODO : improve data display with QModbusValueType ("quamodbusvalue.h")

}

QUaModbusDataBlockWidgetStatus::~QUaModbusDataBlockWidgetStatus()
{
    delete ui;
}

void QUaModbusDataBlockWidgetStatus::setStatus(const QModbusError & status)
{
	auto metaError = QMetaEnum::fromType<QModbusError>();
	auto strError  = QString(metaError.valueToKey(status));
	ui->lineEditStatus->setText(strError);
	// clear table if any error
	if (status != QModbusError::NoError)
	{
		m_modelValues.clear();
	}
}

void QUaModbusDataBlockWidgetStatus::setData(const quint32 &startAddress, const QVector<quint16>& data)
{
	// clear all table
	m_modelValues.setRowCount(data.count());
	// re-populate with new data
	auto parent = m_modelValues.invisibleRootItem();
	for (int row = 0; row < data.count(); row++)
	{
		// set vertical header
		auto header = new QStandardItem(QString("%1").arg(startAddress + row));
		m_modelValues.setVerticalHeaderItem(row, header);
		// set data
		auto item = new QStandardItem(QString("%1").arg(data.at(row)));
		parent->setChild(row, 0, item);
	}
}
