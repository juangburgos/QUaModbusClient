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

	bool                 isIdEditable() const;
	void                 setIdEditable(const bool &idEditable);

	bool                 isTypeEditable() const;
	void                 setTypeEditable(const bool &typeEditable);

	bool                 isAddressEditable() const;
	void                 setAddressEditable(const bool &addressEditable);

	bool                 isSizeEditable() const;
	void                 setSizeEditable(const bool &sizeEditable);

	bool                 isSamplingTimeEditable() const;
	void                 setSamplingTimeEditable(const bool &samplingTimeEditable);
			             
	QString              id() const;
	void                 setId(const QString &strId);

	QModbusDataBlockType type() const;
	void                 setType(const QModbusDataBlockType &type);

	int                  address() const;
	void                 setAddress(const int &address);

	quint32              size() const;
	void                 setSize(const quint32 &size);

	quint32              samplingTime() const;
	void                 setSamplingTime(const quint32 &samplingTime);

private:
    Ui::QUaModbusDataBlockWidgetEdit *ui;
};

#endif // QUAMODBUSDATABLOCKWIDGETEDIT_H
