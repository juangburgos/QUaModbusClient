#ifndef QUAMODBUSVALUEWIDGETEDIT_H
#define QUAMODBUSVALUEWIDGETEDIT_H

#include <QWidget>

#include <QUaModbusValue>

namespace Ui {
class QUaModbusValueWidgetEdit;
}

class QUaModbusValueWidgetEdit : public QWidget
{
    Q_OBJECT

public:
    explicit QUaModbusValueWidgetEdit(QWidget *parent = nullptr);
    ~QUaModbusValueWidgetEdit();

	bool             isIdEditable() const;
	void             setIdEditable(const bool &idEditable);

	bool             isTypeEditable() const;
	void             setTypeEditable(const bool &typeEditable);

	bool             isOffsetEditable() const;
	void             setOffsetEditable(const bool &offsetEditable);

	QString          id() const;
	void             setId(const QString &strId);

	QModbusValueType type() const;
	void             setType(const QModbusValueType &type);

	quint16          offset() const;
	void             setOffset(const quint16 &size);

#ifndef QUAMODBUS_NOCYCLIC_WRITE
	void             setWritable(const bool& writable);

	bool             isCyclicWritePeriodEditable() const;
	void             setCyclicWritePeriodEditable(const bool& editable);

	bool             isCyclicWriteModeEditable() const;
	void             setCyclicWriteModeEditable(const bool& editable);

	quint32                cyclicWritePeriod() const;
	void                   setCyclicWritePeriod(const quint32& cyclicWritePeriod);

	QModbusCyclicWriteMode cyclicWriteMode() const;
	void                   setCyclicWriteMode(const QModbusCyclicWriteMode& cyclicWriteMode);
#endif // !QUAMODBUS_NOCYCLIC_WRITE

private:
    Ui::QUaModbusValueWidgetEdit *ui;
};

#endif // QUAMODBUSVALUEWIDGETEDIT_H
