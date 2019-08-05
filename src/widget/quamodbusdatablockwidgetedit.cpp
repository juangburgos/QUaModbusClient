#include "quamodbusdatablockwidgetedit.h"
#include "ui_quamodbusdatablockwidgetedit.h"

QUaModbusDataBlockWidgetEdit::QUaModbusDataBlockWidgetEdit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QUaModbusDataBlockWidgetEdit)
{
    ui->setupUi(this);
	// setup parity combo
	auto metaBlockType = QMetaEnum::fromType<QModbusDataBlockType>();
	for (int i = 0; i < metaBlockType.keyCount(); i++)
	{
		auto enumBlock = metaBlockType.value(i);
		auto strBlock = QString(metaBlockType.key(i));
		ui->comboBoxType->addItem(strBlock, enumBlock);
	}
	// setup initial values
	this->setType(QModbusDataBlockType::HoldingRegisters);
	this->setAddress(0);
	this->setSize(10);
	this->setSamplingTime(1000);
}

QUaModbusDataBlockWidgetEdit::~QUaModbusDataBlockWidgetEdit()
{
    delete ui;
}

bool QUaModbusDataBlockWidgetEdit::isIdEditable() const
{
	return !ui->lineEditId->isReadOnly();
}

void QUaModbusDataBlockWidgetEdit::setIdEditable(const bool & idEditable)
{
	ui->lineEditId->setReadOnly(!idEditable);
}

bool QUaModbusDataBlockWidgetEdit::isTypeEditable() const
{
	return !ui->comboBoxType->isReadOnly();
}

void QUaModbusDataBlockWidgetEdit::setTypeEditable(const bool & typeEditable)
{
	ui->comboBoxType->setReadOnly(!typeEditable);
}

bool QUaModbusDataBlockWidgetEdit::isAddressEditable() const
{
	return !ui->spinBoxAddress->isReadOnly();
}

void QUaModbusDataBlockWidgetEdit::setAddressEditable(const bool & addressEditable)
{
	ui->spinBoxAddress->setReadOnly(!addressEditable);
}

bool QUaModbusDataBlockWidgetEdit::isSizeEditable() const
{
	return !ui->spinBoxSize->isReadOnly();
}

void QUaModbusDataBlockWidgetEdit::setSizeEditable(const bool & sizeEditable)
{
	ui->spinBoxSize->setReadOnly(!sizeEditable);
}

bool QUaModbusDataBlockWidgetEdit::isSamplingTimeEditable() const
{
	return !ui->spinBoxSampling->isReadOnly();
}

void QUaModbusDataBlockWidgetEdit::setSamplingTimeEditable(const bool & samplingTimeEditable)
{
	ui->spinBoxSampling->setReadOnly(!samplingTimeEditable);
}

QString QUaModbusDataBlockWidgetEdit::id() const
{
	return ui->lineEditId->text();
}

void QUaModbusDataBlockWidgetEdit::setId(const QString & strId)
{
	ui->lineEditId->setText(strId);
}

QModbusDataBlockType QUaModbusDataBlockWidgetEdit::type() const
{
	return ui->comboBoxType->currentData().value<QModbusDataBlockType>();
}

void QUaModbusDataBlockWidgetEdit::setType(const QModbusDataBlockType & type)
{
	auto strType = QString(QMetaEnum::fromType<QModbusDataBlockType>().valueToKey(type));
	Q_ASSERT(ui->comboBoxType->findText(strType) >= 0);
	ui->comboBoxType->setCurrentText(strType);
}

int QUaModbusDataBlockWidgetEdit::address() const
{
	return ui->spinBoxAddress->value();
}

void QUaModbusDataBlockWidgetEdit::setAddress(const int & address)
{
	ui->spinBoxAddress->setValue(address);
}

quint32 QUaModbusDataBlockWidgetEdit::size() const
{
	return ui->spinBoxSize->value();
}

void QUaModbusDataBlockWidgetEdit::setSize(const quint32 & size)
{
	ui->spinBoxSize->setValue(size);
}

quint32 QUaModbusDataBlockWidgetEdit::samplingTime() const
{
	return ui->spinBoxSampling->value();
}

void QUaModbusDataBlockWidgetEdit::setSamplingTime(const quint32 & samplingTime)
{
	ui->spinBoxSampling->setValue(samplingTime);
}
