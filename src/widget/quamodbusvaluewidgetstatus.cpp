#define NOMINMAX

#include "quamodbusvaluewidgetstatus.h"
#include "ui_quamodbusvaluewidgetstatus.h"

#include <limits>

QUaModbusValueWidgetStatus::QUaModbusValueWidgetStatus(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QUaModbusValueWidgetStatus)
{
    ui->setupUi(this);
	m_valueOld  = 0;
	m_valueCurr = 0;
	// limits
	ui->spinBoxValue->setMinimum(std::numeric_limits<int>::min());
	ui->spinBoxValue->setMaximum(std::numeric_limits<int>::max());
	ui->doubleSpinBoxValue->setMinimum(-std::numeric_limits<double>::infinity());
	ui->doubleSpinBoxValue->setMaximum(+std::numeric_limits<double>::infinity());
	// checkbox read only
	ui->checkBoxValue->setAttribute(Qt::WA_TransparentForMouseEvents, true);
	ui->checkBoxValue->setFocusPolicy(Qt::NoFocus);
	// hide all
	ui->checkBoxValue->setEnabled(false);
	ui->checkBoxValue->setVisible(false);
	ui->spinBoxValue->setEnabled(false);
	ui->spinBoxValue->setVisible(false);
	ui->doubleSpinBoxValue->setEnabled(false);
	ui->doubleSpinBoxValue->setVisible(false);
}

QUaModbusValueWidgetStatus::~QUaModbusValueWidgetStatus()
{
    delete ui;
}

bool QUaModbusValueWidgetStatus::isFrozen() const
{
	return ui->checkBoxFreeze->isChecked();
}

void QUaModbusValueWidgetStatus::setIsFrozen(const bool & frozen)
{
	ui->checkBoxFreeze->setChecked(frozen);
}

void QUaModbusValueWidgetStatus::setType(const QModbusValueType & type)
{
	// hide all
	ui->checkBoxValue->setEnabled(false);
	ui->checkBoxValue->setVisible(false);
	ui->spinBoxValue->setEnabled(false);
	ui->spinBoxValue->setVisible(false);
	ui->doubleSpinBoxValue->setEnabled(false);
	ui->doubleSpinBoxValue->setVisible(false);
	// show specific
	switch (type)
	{
	case QModbusValueType::Binary0:
	case QModbusValueType::Binary1:
	case QModbusValueType::Binary2:
	case QModbusValueType::Binary3:
	case QModbusValueType::Binary4:
	case QModbusValueType::Binary5:
	case QModbusValueType::Binary6:
	case QModbusValueType::Binary7:
	case QModbusValueType::Binary8:
	case QModbusValueType::Binary9:
	case QModbusValueType::Binary10:
	case QModbusValueType::Binary11:
	case QModbusValueType::Binary12:
	case QModbusValueType::Binary13:
	case QModbusValueType::Binary14:
	case QModbusValueType::Binary15:
		{
			ui->checkBoxValue->setEnabled(true);
			ui->checkBoxValue->setVisible(true);
			break;
		}
	case QModbusValueType::Decimal:
	case QModbusValueType::Int:
	case QModbusValueType::IntSwapped:
	case QModbusValueType::Int64:
	case QModbusValueType::Int64Swapped:
	case QModbusValueType::Invalid: // NOTE : invalid
		{
			ui->spinBoxValue->setEnabled(true);
			ui->spinBoxValue->setVisible(true);
			break;
		}
	case QModbusValueType::Float:
	case QModbusValueType::FloatSwapped:
	case QModbusValueType::Float64:
	case QModbusValueType::Float64Swapped:
		{
			ui->doubleSpinBoxValue->setEnabled(true);
			ui->doubleSpinBoxValue->setVisible(true);
			break;
		}
	default:
		{
			Q_ASSERT(false);
			break;
		}
	}
}

void QUaModbusValueWidgetStatus::setStatus(const QModbusError & status)
{
	auto metaError = QMetaEnum::fromType<QModbusError>();
	auto strError  = QString(metaError.valueToKey(status));
	ui->lineEditStatus->setText(strError);
}

void QUaModbusValueWidgetStatus::setRegistersUsed(const quint16 & registersUsed)
{
	ui->spinBoxRegUsed->setValue(registersUsed);
}

void QUaModbusValueWidgetStatus::setData(const QVector<quint16>& data)
{
	QString strData;
	for (int i = 0; i < data.count(); i++)
	{
		strData += (i == 0 ? "[" : ", ") + QString("%1").arg(data.at(i)) + (i == data.count()-1 ? "]" : "");
	}
	ui->lineEditData->setText(strData);
}

QVariant QUaModbusValueWidgetStatus::value() const
{
	return m_valueCurr;
}

void QUaModbusValueWidgetStatus::setValue(const QVariant & value)
{
	if (this->isFrozen() || value == m_valueOld)
	{
		return;
	}
	// NOTE : only update if necessary, so user can input a new value
	if (ui->checkBoxValue->isEnabled())
	{
		auto newVal = value.toBool();
		ui->checkBoxValue->setChecked(newVal);
		ui->checkBoxValue->setText(newVal ? tr("True") : tr("False"));
	}
	else if (ui->spinBoxValue->isEnabled())
	{
		auto newVal = value.toLongLong();
		ui->spinBoxValue->setValue(newVal);
	}
	else if (ui->doubleSpinBoxValue->isEnabled())
	{
		auto newVal = value.toDouble();
		ui->doubleSpinBoxValue->setValue(newVal);
	}
	else
	{
		Q_ASSERT(false);
	}
	// update internal values
	m_valueOld  = value;
	m_valueCurr = value;
}

void QUaModbusValueWidgetStatus::setWritable(const bool & writable)
{
	ui->checkBoxValue->setAttribute(Qt::WA_TransparentForMouseEvents, !writable);
	ui->spinBoxValue->setReadOnly(!writable);
	ui->doubleSpinBoxValue->setReadOnly(!writable);
}

void QUaModbusValueWidgetStatus::on_checkBoxValue_stateChanged(int arg1)
{
	Q_UNUSED(arg1);
	m_valueCurr = ui->checkBoxValue->isChecked();
	emit this->valueUpdated(m_valueCurr);
}

void QUaModbusValueWidgetStatus::on_spinBoxValue_valueChanged(int arg1)
{
	m_valueCurr = arg1;
	emit this->valueUpdated(m_valueCurr);
}

void QUaModbusValueWidgetStatus::on_doubleSpinBoxValue_valueChanged(double arg1)
{
	m_valueCurr = arg1;
	emit this->valueUpdated(m_valueCurr);
}
