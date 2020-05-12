#include "quamodbusdatablockwidgetstatus.h"
#include "ui_quamodbusdatablockwidgetstatus.h"

QUaModbusDataBlockWidgetStatus::QUaModbusDataBlockWidgetStatus(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QUaModbusDataBlockWidgetStatus)
{
    ui->setupUi(this);
	m_lastStartAddress = 999999;
	// setup params table
	ui->tableViewData->setModel(&m_modelValues);
	ui->tableViewData->setAlternatingRowColors(true);
	ui->tableViewData->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->tableViewData->horizontalHeader()->setStretchLastSection(true);
	ui->tableViewData->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->tableViewData->setEditTriggers(QAbstractItemView::NoEditTriggers);
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
	// NOTE : do not clear table if any error
}

void QUaModbusDataBlockWidgetStatus::setData(const quint32 &startAddress, const QVector<quint16>& data)
{
	// NOTE : only arrive here if no error
	auto parent = m_modelValues.invisibleRootItem();
	// check if need to resize table
	int rowCount = m_modelValues.rowCount();
	if (rowCount != data.count())
	{
		// clear all table
		m_modelValues.removeRows(0, m_modelValues.rowCount());
		// NOTE : it is much more performant to pre-allocate the entire row in advance,
		//        and calling parent->appendRow (calling parent->setChild is expensive)
		QList<QStandardItem*> listCols;
		std::generate_n(std::back_inserter(listCols), data.count(),
		[]() {
			return new QStandardItem;
		});
		parent->appendRows(listCols);
	}
	// check if need to reset vertical headers
	if (m_lastStartAddress != startAddress)
	{
		m_lastStartAddress = startAddress;
		// reset vertical headers
		for (int row = 0; row < data.count(); row++)
		{
			// set vertical header
			auto header = new QStandardItem(QString("%1").arg(startAddress + row));
			m_modelValues.setVerticalHeaderItem(row, header);
		}
	}
	// re-populate with new data
	for (int row = 0; row < data.count(); row++)
	{
		// set data
		auto item = parent->child(row, 0);
		item->setText(QString("%1").arg(data.at(row)));
	}
}
