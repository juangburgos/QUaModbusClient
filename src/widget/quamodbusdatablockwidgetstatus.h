#ifndef QUAMODBUSDATABLOCKWIDGETSTATUS_H
#define QUAMODBUSDATABLOCKWIDGETSTATUS_H

#include <QWidget>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>

#include <QUaModbusDataBlock>

namespace Ui {
class QUaModbusDataBlockWidgetStatus;
}

class QUaModbusDataBlockWidgetStatus : public QWidget
{
    Q_OBJECT

public:
    explicit QUaModbusDataBlockWidgetStatus(QWidget *parent = nullptr);
    ~QUaModbusDataBlockWidgetStatus();

	void setStatus(const QModbusError &status);

	void setData(const quint32 &startAddress, const QVector<quint16> &data);

private:
    Ui::QUaModbusDataBlockWidgetStatus *ui;
	QStandardItemModel    m_modelValues;
	quint32 m_lastStartAddress;
};

#endif // QUAMODBUSDATABLOCKWIDGETSTATUS_H
