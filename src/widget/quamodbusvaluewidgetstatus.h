#ifndef QUAMODBUSVALUEWIDGETSTATUS_H
#define QUAMODBUSVALUEWIDGETSTATUS_H

#include <QWidget>

#include <QUaModbusValue>

namespace Ui {
class QUaModbusValueWidgetStatus;
}

class QUaModbusValueWidgetStatus : public QWidget
{
    Q_OBJECT

public:
    explicit QUaModbusValueWidgetStatus(QWidget *parent = nullptr);
    ~QUaModbusValueWidgetStatus();

	bool isFrozen() const;
	void setIsFrozen(const bool &frozen);

	void setType(const QModbusValueType &type);

	void setStatus(const QModbusError &status);

	void setRegistersUsed(const quint16 &registersUsed);

	void setData(const QVector<quint16> &data);

	QVariant value() const;
	void     setValue(const QVariant &value);

	void setWritable(const bool &writable);

signals:
	void valueUpdated(const QVariant &value);

private slots:
    void on_checkBoxValue_stateChanged(int arg1);

    void on_spinBoxValue_valueChanged(int arg1);

    void on_doubleSpinBoxValue_valueChanged(double arg1);

private:
    Ui::QUaModbusValueWidgetStatus *ui;
	QVariant m_valueOld;
	QVariant m_valueCurr;
};

#endif // QUAMODBUSVALUEWIDGETSTATUS_H
