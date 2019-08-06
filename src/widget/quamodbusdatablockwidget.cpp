#include "quamodbusdatablockwidget.h"
#include "ui_quamodbusdatablockwidget.h"

#include <QMessageBox>

#include <QUaModbusDataBlock>

#include <QUaModbusClientDialog>
#include <QUaModbusValueWidgetEdit>

#ifdef QUA_ACCESS_CONTROL
#include <QUaDockWidgetPerms>
#endif // QUA_ACCESS_CONTROL

QUaModbusDataBlockWidget::QUaModbusDataBlockWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QUaModbusDataBlockWidget)
{
    ui->setupUi(this);
#ifndef QUA_ACCESS_CONTROL
	ui->pushButtonPerms->setVisible(false);
#endif // !QUA_ACCESS_CONTROL
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
	// check if valid
	if (!block)
	{
		this->setEnabled(false);
		return;
	}
	// bind common
	m_connections <<
	QObject::connect(block, &QObject::destroyed, this,
	[this]() {
		this->bindBlock(nullptr);
	});
	// enable
	this->setEnabled(true);
	// bind edit widget
	this->bindBlockWidgetEdit(block);
	// bind status widget
	this->bindBlockWidgetStatus(block);
	// bind buttons
#ifdef QUA_ACCESS_CONTROL
	m_connections <<
	QObject::connect(ui->pushButtonPerms, &QPushButton::clicked, block,
	[this, block]() {
		// NOTE : call QUaModbusClientWidget::setupPermissionsModel first to set m_proxyPerms
		Q_CHECK_PTR(m_proxyPerms);
		// create permissions widget
		auto permsWidget = new QUaDockWidgetPerms;
		// configure perms widget combo
		permsWidget->setComboModel(m_proxyPerms);
		permsWidget->setPermissions(block->permissionsObject());
		// dialog
		QUaModbusClientDialog dialog(this);
		dialog.setWindowTitle(tr("Modbus Block Permissions"));
		dialog.setWidget(permsWidget);
		// exec dialog
		int res = dialog.exec();
		if (res != QDialog::Accepted)
		{
			return;
		}
		// read permissions and set them for layout list
		block->setPermissionsObject(permsWidget->permissions());
	});
#endif // QUA_ACCESS_CONTROL
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
	m_connections <<
	QObject::connect(ui->pushButtonClear, &QPushButton::clicked, block,
	[this, block]() {
		Q_CHECK_PTR(block);
		// are you sure?
		auto res = QMessageBox::question(
			this,
			tr("Delete All Values Confirmation"),
			tr("Are you sure you want to delete all values for block %1?\n").arg(block->browseName()),
			QMessageBox::StandardButton::Ok,
			QMessageBox::StandardButton::Cancel
		);
		if (res != QMessageBox::StandardButton::Ok)
		{
			return;
		}
		// clear
		block->values()->clear();
	});
	// NOTE : apply button bound in bindBlockWidgetEdit
}

void QUaModbusDataBlockWidget::clear()
{
	// disable old connections
	while (m_connections.count() > 0)
	{
		QObject::disconnect(m_connections.takeFirst());
	}
	// clear edit widget
	ui->widgetBlockEdit->setId("");
	// clear status widget
}

#ifdef QUA_ACCESS_CONTROL
void QUaModbusDataBlockWidget::setupPermissionsModel(QSortFilterProxyModel * proxyPerms)
{
	m_proxyPerms = proxyPerms;
	Q_CHECK_PTR(m_proxyPerms);
}

void QUaModbusDataBlockWidget::setCanWrite(const bool & canWrite)
{
	ui->widgetBlockEdit->setTypeEditable(canWrite);
	ui->widgetBlockEdit->setAddressEditable(canWrite);
	ui->widgetBlockEdit->setSizeEditable(canWrite);
	ui->widgetBlockEdit->setSamplingTimeEditable(canWrite);
	ui->pushButtonApply->setEnabled(canWrite);
	ui->pushButtonDelete->setVisible(canWrite);
	ui->pushButtonClear->setVisible(canWrite);
}

void QUaModbusDataBlockWidget::setCanWriteBlockList(const bool & canWrite)
{
	ui->pushButtonPerms->setVisible(canWrite);
}

void QUaModbusDataBlockWidget::setCanWriteValueList(const bool & canWrite)
{
	ui->pushButtonAddValue->setVisible(canWrite);
}
#endif // QUA_ACCESS_CONTROL

void QUaModbusDataBlockWidget::bindBlockWidgetEdit(QUaModbusDataBlock * block)
{
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
