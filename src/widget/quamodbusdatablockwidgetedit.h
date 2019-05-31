#ifndef QUAMODBUSDATABLOCKWIDGETEDIT_H
#define QUAMODBUSDATABLOCKWIDGETEDIT_H

#include <QWidget>

#include <QUaModbusDataBlock>

namespace Ui {
class QUaModbusDataBlockWidgetEdit;
}

class QUaModbusDataBlockWidgetEdit : public QWidget
{
    Q_OBJECT

public:
    explicit QUaModbusDataBlockWidgetEdit(QWidget *parent = nullptr);
    ~QUaModbusDataBlockWidgetEdit();

	QString          id() const;
	void             strId(const QString &strId);

	QUaModbusDataBlockType type() const;
	void                   setType(const QUaModbusDataBlockType &type);

	int              address() const;
	void             setAddress(const int &address);

	quint32          size() const;
	void             setSize(const quint32 &size);

	quint32          samplingTime() const;
	void             setSamplingTime(const quint32 &samplingTime);

private:
    Ui::QUaModbusDataBlockWidgetEdit *ui;
};

#endif // QUAMODBUSDATABLOCKWIDGETEDIT_H
