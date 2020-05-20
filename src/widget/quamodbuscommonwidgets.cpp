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

/*---------------------------------------------------------------------------------------------
*/

QUaModbusLambdaFilterProxy::QUaModbusLambdaFilterProxy(QObject *parent/* = 0*/)
	: QSortFilterProxyModel(parent)
{

}

//int QUaModbusLambdaFilterProxy::columnCount(const QModelIndex& parent) const
//{
//	auto source = this->sourceModel();
//	int colCount = source ? source->columnCount(parent) : 0;
//	return colCount;
//}


void QUaModbusLambdaFilterProxy::resetFilter()
{
	this->invalidateFilter();
}
bool QUaModbusLambdaFilterProxy::filterAcceptsRow(int sourceRow, const QModelIndex & sourceParent) const
{
	return m_filterAcceptsRow ? m_filterAcceptsRow(sourceRow, sourceParent) : QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
}

bool QUaModbusLambdaFilterProxy::lessThan(const QModelIndex & left, const QModelIndex & right) const
{
	return m_lessThan ? m_lessThan(left, right) : QSortFilterProxyModel::lessThan(left, right);
}
