#include "quamodbuscommonwidgets.h"

QUaModbusReadOnlyComboBox::QUaModbusReadOnlyComboBox(QWidget* parent/* = Q_NULLPTR*/)
	: QComboBox(parent)
{
	m_bReadOnly = false;
}

bool QUaModbusReadOnlyComboBox::isReadOnly() const
{
	return m_bReadOnly;
}

void QUaModbusReadOnlyComboBox::setReadOnly(const bool & bReadOnly)
{
	m_bReadOnly = bReadOnly;
}

void QUaModbusReadOnlyComboBox::mousePressEvent(QMouseEvent * e)
{
	if (m_bReadOnly)
	{
		return;
	}
	QComboBox::mousePressEvent(e);
}

void QUaModbusReadOnlyComboBox::keyPressEvent(QKeyEvent * e)
{
	if (m_bReadOnly)
	{
		return;
	}
	QComboBox::keyPressEvent(e);
}

void QUaModbusReadOnlyComboBox::wheelEvent(QWheelEvent * e)
{
	if (m_bReadOnly)
	{
		return;
	}
	QComboBox::wheelEvent(e);
}