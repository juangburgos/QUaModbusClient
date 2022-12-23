#include "quamodbusvaluewidgetedit.h"
#include "ui_quamodbusvaluewidgetedit.h"

#include <QUaWidgetEventFilter>

QUaModbusValueWidgetEdit::QUaModbusValueWidgetEdit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QUaModbusValueWidgetEdit)
{
    ui->setupUi(this);
    QUaWidgetEventFilterCallback blockWheel = [](const QEvent* event) {
		Q_UNUSED(event);
		return true;
	};
	// setup type combo
	auto metaValueType = QMetaEnum::fromType<QModbusValueType>();
	for (int i = 0; i < metaValueType.keyCount(); i++)
	{
		auto enumValue = metaValueType.value(i);
		auto strValue  = QString(metaValueType.key(i));
		ui->comboBoxType->addItem(strValue, enumValue);
	}
	// setup initial values
	this->setType(QModbusValueType::Decimal);
	this->setOffset(0);
#ifndef QUAMODBUS_NOCYCLIC_WRITE
	// setup mode combo
	auto metaCyclicMode = QMetaEnum::fromType<QModbusCyclicWriteMode>();
	for (int i = 0; i < metaCyclicMode.keyCount(); i++)
	{
		auto enumValue = metaCyclicMode.value(i);
		auto strValue = QString(metaCyclicMode.key(i));
		ui->comboBoxCyclicMode->addItem(strValue, enumValue);
	}
	// setup initial values
	this->setCyclicWriteMode(QModbusCyclicWriteMode::Current);
	this->setCyclicWritePeriod(0);
#else
	ui->frameCyclic->setVisible(false);
	ui->frameCyclic->setEnabled(false);
#endif // !QUAMODBUS_NOCYCLIC_WRITE
	// block wheel on widgets
	auto comboBoxTypeEventHandler = new QUaWidgetEventFilter(ui->comboBoxType);
	comboBoxTypeEventHandler->installEventCallback(QEvent::Wheel, blockWheel);
	auto comboBoxCyclicModeEventHandler = new QUaWidgetEventFilter(ui->comboBoxCyclicMode);
	comboBoxCyclicModeEventHandler->installEventCallback(QEvent::Wheel, blockWheel);
	auto spinBoxOffsetEventHandler = new QUaWidgetEventFilter(ui->spinBoxOffset);
	spinBoxOffsetEventHandler->installEventCallback(QEvent::Wheel, blockWheel);
	auto spinBoxCyclicPeriodEventHandler = new QUaWidgetEventFilter(ui->spinBoxCyclicPeriod);
	spinBoxCyclicPeriodEventHandler->installEventCallback(QEvent::Wheel, blockWheel);
}

QUaModbusValueWidgetEdit::~QUaModbusValueWidgetEdit()
{
    delete ui;
}

bool QUaModbusValueWidgetEdit::isIdEditable() const
{
	return !ui->lineEditId->isReadOnly();
}

void QUaModbusValueWidgetEdit::setIdEditable(const bool & idEditable)
{
	ui->lineEditId->setReadOnly(!idEditable);
}

bool QUaModbusValueWidgetEdit::isTypeEditable() const
{
	return !ui->comboBoxType->isReadOnly();
}

void QUaModbusValueWidgetEdit::setTypeEditable(const bool & typeEditable)
{
	ui->comboBoxType->setReadOnly(!typeEditable);
}

bool QUaModbusValueWidgetEdit::isOffsetEditable() const
{
	return !ui->spinBoxOffset->isReadOnly();
}

void QUaModbusValueWidgetEdit::setOffsetEditable(const bool & offsetEditable)
{
	ui->spinBoxOffset->setReadOnly(!offsetEditable);
}

QString QUaModbusValueWidgetEdit::id() const
{
	return ui->lineEditId->text();
}

void QUaModbusValueWidgetEdit::setId(const QString & strId)
{
	ui->lineEditId->setText(strId);
}

QModbusValueType QUaModbusValueWidgetEdit::type() const
{
	return ui->comboBoxType->currentData().value<QModbusValueType>();
}

void QUaModbusValueWidgetEdit::setType(const QModbusValueType & type)
{
	auto strType = QString(QMetaEnum::fromType<QModbusValueType>().valueToKey(type));
	Q_ASSERT(ui->comboBoxType->findText(strType) >= 0);
	ui->comboBoxType->setCurrentText(strType);
}

quint16 QUaModbusValueWidgetEdit::offset() const
{
	return ui->spinBoxOffset->value();
}

void QUaModbusValueWidgetEdit::setOffset(const quint16 & size)
{
	ui->spinBoxOffset->setValue(size);
}

#ifndef QUAMODBUS_NOCYCLIC_WRITE
void QUaModbusValueWidgetEdit::setWritable(const bool& writable)
{
	ui->frameCyclic->setVisible(writable);
	ui->frameCyclic->setEnabled(writable);
}

bool QUaModbusValueWidgetEdit::isCyclicWritePeriodEditable() const
{
	return !ui->spinBoxCyclicPeriod->isReadOnly();
}

void QUaModbusValueWidgetEdit::setCyclicWritePeriodEditable(const bool& editable)
{
	ui->spinBoxCyclicPeriod->setReadOnly(!editable);
}

bool QUaModbusValueWidgetEdit::isCyclicWriteModeEditable() const
{
	return !ui->comboBoxCyclicMode->isReadOnly();
}

void QUaModbusValueWidgetEdit::setCyclicWriteModeEditable(const bool& editable)
{
	ui->spinBoxOffset->setReadOnly(!editable);
}

quint32 QUaModbusValueWidgetEdit::cyclicWritePeriod() const
{
	return ui->spinBoxCyclicPeriod->value();
}

void QUaModbusValueWidgetEdit::setCyclicWritePeriod(const quint32& cyclicWritePeriod)
{
	ui->spinBoxCyclicPeriod->setValue(cyclicWritePeriod);
}

QModbusCyclicWriteMode QUaModbusValueWidgetEdit::cyclicWriteMode() const
{
	return ui->comboBoxCyclicMode->currentData().value<QModbusCyclicWriteMode>();
}

void QUaModbusValueWidgetEdit::setCyclicWriteMode(const QModbusCyclicWriteMode& cyclicWriteMode)
{
	auto strMode = QString(QMetaEnum::fromType<QModbusCyclicWriteMode>().valueToKey(cyclicWriteMode));
	Q_ASSERT(ui->comboBoxCyclicMode->findText(strMode) >= 0);
	ui->comboBoxCyclicMode->setCurrentText(strMode);
}
#endif // !QUAMODBUS_NOCYCLIC_WRITE
