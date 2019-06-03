#include "quamodbusvaluewidgetedit.h"
#include "ui_quamodbusvaluewidgetedit.h"

QUaModbusValueWidgetEdit::QUaModbusValueWidgetEdit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QUaModbusValueWidgetEdit)
{
    ui->setupUi(this);
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
}

QUaModbusValueWidgetEdit::~QUaModbusValueWidgetEdit()
{
    delete ui;
}

bool QUaModbusValueWidgetEdit::isIdEditable() const
{
	return ui->lineEditId->isEnabled();
}

void QUaModbusValueWidgetEdit::setIdEditable(const bool & idEditable)
{
	ui->lineEditId->setEnabled(idEditable);
}

QString QUaModbusValueWidgetEdit::id() const
{
	return ui->lineEditId->text();
}

void QUaModbusValueWidgetEdit::strId(const QString & strId)
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
