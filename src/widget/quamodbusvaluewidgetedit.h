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

private:
    Ui::QUaModbusValueWidgetEdit *ui;
};

#endif // QUAMODBUSVALUEWIDGETEDIT_H
