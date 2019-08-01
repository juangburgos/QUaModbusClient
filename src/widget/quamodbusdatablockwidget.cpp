#include "quamodbusdatablockwidget.h"
#include "ui_quamodbusdatablockwidget.h"

#include <QMessageBox>

#include <QUaModbusDataBlock>

#include <QUaModbusClientDialog>
#include <QUaModbusValueWidgetEdit>

QUaModbusDataBlockWidget::QUaModbusDataBlockWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QUaModbusDataBlockWidget)
{
    ui->setupUi(this);
}

QUaModbusDataBlockWidget::~QUaModbusDataBlockWidget()
{
    delete ui;
}

void QUaModbusDataBlockWidget::bindBlock(QUaModbusDataBlock * block)
{
	// disable old connections
	while (m_connections.count() > 0)
	{
		QObject::disconnect(m_connections.takeFirst());
	}
	// bind edit widget
	this->bindBlockWidgetEdit(block);
	// bind status widget
	this->bindBlockWidgetStatus(block);
	// bind buttons
	m_connections <<
	QObject::connect(ui->pushButtonAddValue, &QPushButton::clicked, block,
	[this, block]() {
		Q_CHECK_PTR(block);
		// use value edit widget
		QUaModbusValueWidgetEdit * widgetNewValue = new QUaModbusValueWidgetEdit;
		QUaModbusClientDialog dialog(this);
		dialog.setWindowTitle(tr("New Modbus Value"));
		// NOTE : dialog takes ownershit
		dialog.setWidget(widgetNewValue);
		// NOTE : call in own method to we can recall it if fails
		this->showNewValueDialog(block, dialog);
	});
	m_connections <<
	QObject::connect(ui->pushButtonDelete, &QPushButton::clicked, block,
	[this, block]() {
		Q_CHECK_PTR(block);
		// are you sure?
		auto res = QMessageBox::question(
			this,
			tr("Delete Block Confirmation"),
			tr("Deleting block %1 will also delete all its Values.\nWould you like to delete block %1?").arg(block->browseName()),
			QMessageBox::StandardButton::Ok,
			QMessageBox::StandardButton::Cancel
		);
		if (res != QMessageBox::StandardButton::Ok)
		{
			return;
		}
		// delete
		block->remove();
		// NOTE : removed from tree on &QObject::destroyed callback
	});
	// NOTE : apply button bound in bindBlockWidgetEdit
}

void QUaModbusDataBlockWidget::bindBlockWidgetEdit(QUaModbusDataBlock * block)
{
	// enable edit widget
	ui->widgetBlockEdit->setEnabled(true);
	// bind
	m_connections <<
	QObject::connect(block, &QObject::destroyed, ui->widgetBlockEdit,
	[this]() {
		ui->widgetBlockEdit->setEnabled(false);
	});
	// id
	ui->widgetBlockEdit->setIdEditable(false);
	ui->widgetBlockEdit->setId(block->browseName());
	// type
	ui->widgetBlockEdit->setType(block->getType());
	m_connections <<
	QObject::connect(block, &QUaModbusDataBlock::typeChanged, ui->widgetBlockEdit,
	[this](const QModbusDataBlockType &type) {
		ui->widgetBlockEdit->setType(type);
	});
	// address
	ui->widgetBlockEdit->setAddress(block->getAddress());
	m_connections <<
	QObject::connect(block, &QUaModbusDataBlock::addressChanged, ui->widgetBlockEdit,
	[this](const int &address) {
		ui->widgetBlockEdit->setAddress(address);
	});
	// size
	ui->widgetBlockEdit->setSize(block->getSize());
	m_connections <<
	QObject::connect(block, &QUaModbusDataBlock::sizeChanged, ui->widgetBlockEdit,
	[this](const quint32 &size) {
		ui->widgetBlockEdit->setSize(size);
	});
	// sampling
	ui->widgetBlockEdit->setSamplingTime(block->getSamplingTime());
	m_connections <<
	QObject::connect(block, &QUaModbusDataBlock::samplingTimeChanged, ui->widgetBlockEdit,
	[this](const quint32 &samplingTime) {
		ui->widgetBlockEdit->setSamplingTime(samplingTime);
	});
	// on apply
	m_connections <<
	QObject::connect(ui->pushButtonApply, &QPushButton::clicked, ui->widgetBlockEdit,
	[block, this]() {
		block->setType        (ui->widgetBlockEdit->type());
		block->setAddress     (ui->widgetBlockEdit->address());
		block->setSize        (ui->widgetBlockEdit->size());
		block->setSamplingTime(ui->widgetBlockEdit->samplingTime());
	});
}

void QUaModbusDataBlockWidget::bindBlockWidgetStatus(QUaModbusDataBlock * block)
{
	// enable status widget
	ui->widgetBlockStatus->setEnabled(true);
	// bind
	m_connections <<
	QObject::connect(block, &QObject::destroyed, ui->widgetBlockStatus,
	[this]() {
		ui->widgetBlockStatus->setEnabled(false);
	});
	// status
	ui->widgetBlockStatus->setStatus(block->getLastError());
	m_connections <<
	QObject::connect(block, &QUaModbusDataBlock::lastErrorChanged, ui->widgetBlockStatus,
	[this](const QModbusError & error) {
		ui->widgetBlockStatus->setStatus(error);
	});
	// data
	ui->widgetBlockStatus->setData(0, block->getData());
	m_connections <<
	QObject::connect(block, &QUaModbusDataBlock::dataChanged, ui->widgetBlockStatus,
	[this](const QVector<quint16> & data) {
		ui->widgetBlockStatus->setData(0, data);
	});
}

void QUaModbusDataBlockWidget::showNewValueDialog(QUaModbusDataBlock * block, QUaModbusClientDialog & dialog)
{
	Q_CHECK_PTR(block);
	int res = dialog.exec();
	if (res != QDialog::Accepted)
	{
		return;
	}
	// get new client type
	auto widgetNewValue = qobject_cast<QUaModbusValueWidgetEdit*>(dialog.widget());
	Q_CHECK_PTR(widgetNewValue);
	// get data from widget
	auto strValueId = widgetNewValue->id();
	// check
	auto listValues  = block->values();
	QString strError = listValues->addValue(strValueId);
	if (strError.contains("Error", Qt::CaseInsensitive))
	{
		QMessageBox::critical(this, tr("New Value Error"), strError, QMessageBox::StandardButton::Ok);
		this->showNewValueDialog(block, dialog);
		return;
	}
	// set properties
	auto value = listValues->browseChild<QUaModbusValue>(strValueId);
	Q_CHECK_PTR(value);
	value->setType(widgetNewValue->type());
	value->setAddressOffset(widgetNewValue->offset());
	// NOTE : new value is added to tree using OPC UA events 
}
