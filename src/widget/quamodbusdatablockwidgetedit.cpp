#include "quamodbusdatablockwidgetedit.h"
#include "ui_quamodbusdatablockwidgetedit.h"

QUaModbusDataBlockWidgetEdit::QUaModbusDataBlockWidgetEdit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QUaModbusDataBlockWidgetEdit)
{
    ui->setupUi(this);
	// setup parity combo
	auto metaBlockType = QMetaEnum::fromType<QUaModbusDataBlockType>();
	for (int i = 0; i < metaBlockType.keyCount(); i++)
	{
		auto enumBlock = metaBlockType.value(i);
		auto strBlock = QString(metaBlockType.key(i));
		ui->comboBoxType->addItem(strBlock, enumBlock);
	}
	// setup initial values
	this->setType(QUaModbusDataBlockType::HoldingRegisters);
	this->setAddress(0);
	this->setSize(10);
	this->setSamplingTime(1000);
}

QUaModbusDataBlockWidgetEdit::~QUaModbusDataBlockWidgetEdit()
{
    delete ui;
}

QString QUaModbusDataBlockWidgetEdit::id() const
{
	return ui->lineEditId->text();
}

void QUaModbusDataBlockWidgetEdit::strId(const QString & strId)
{
	ui->lineEditId->setText(strId);
}

QUaModbusDataBlockType QUaModbusDataBlockWidgetEdit::type() const
{
	return ui->comboBoxType->currentData().value<QUaModbusDataBlockType>();
}

void QUaModbusDataBlockWidgetEdit::setType(const QUaModbusDataBlockType & type)
{
	auto strType = QString(QMetaEnum::fromType<QUaModbusDataBlockType>().valueToKey(type));
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
