#ifndef QUAMODBUSVALUE_H
#define QUAMODBUSVALUE_H

#include <QModbusDataUnit>
#include <QModbusReply>

#include <QUaBaseObject>

class QUaModbusDataBlock;
class QUaModbusValueList;

class QUaModbusValue : public QUaBaseObject
{
	friend class QUaModbusValueList;
	friend class QUaModbusDataBlock;

    Q_OBJECT

	// UA properties
	Q_PROPERTY(QUaProperty * Type          READ type         )
	Q_PROPERTY(QUaProperty * RegistersUsed READ registersUsed)
	Q_PROPERTY(QUaProperty * AddressOffset READ addressOffset)

	// UA variables
	Q_PROPERTY(QUaBaseDataVariable * Value     READ value    )
	Q_PROPERTY(QUaBaseDataVariable * LastError READ lastError)

public:
	Q_INVOKABLE explicit QUaModbusValue(QUaServer *server);

	enum ValueType
	{
		Invalid        = -1,
		Binary0        = 0,
		Binary1        = 1,
		Binary2        = 2,
		Binary3        = 3,
		Binary4        = 4,
		Binary5        = 5,
		Binary6        = 6,
		Binary7        = 7,
		Binary8        = 8,
		Binary9        = 9,
		Binary10       = 10,
		Binary11       = 11,
		Binary12       = 12,
		Binary13       = 13,
		Binary14       = 14,
		Binary15       = 15,
		Decimal        = 16,
		Int            = 17, // i32 Least Significant Register First
		IntSwapped     = 18, // i32 Most Significant Register First
		Float          = 19, // f32 Least Significant Register First
		FloatSwapped   = 20, // f32 Most Significant Register First
		Int64          = 21, // i64 Least Significant Register First
		Int64Swapped   = 22, // i64 Most Significant Register First
		Float64        = 23, // f64 Least Significant Register First
		Float64Swapped = 24, // f64 Most Significant Register First
	};
	Q_ENUM(ValueType)

	// UA properties

	QUaProperty * type();
	QUaProperty * registersUsed();
	QUaProperty * addressOffset();

	// UA variables

	QUaBaseDataVariable * value();
	QUaBaseDataVariable * lastError();

	// UA methods

	Q_INVOKABLE void remove();

private slots:
	void on_typeChanged         (const QVariant &value);
	void on_addressOffsetChanged(const QVariant &value);
	void on_valueChanged        (const QVariant &value);

private:

	QUaModbusDataBlock * block();

	void setValue(const QVector<quint16> &block, const QModbusDevice::Error &blockError);

	static int              typeBlockSize(const QUaModbusValue::ValueType &type);
	static QMetaType::Type  typeToMeta   (const QUaModbusValue::ValueType &type);
	static QVariant         blockToValue (const QVector<quint16> &block, const QUaModbusValue::ValueType &type);
	static QVector<quint16> valueToBlock (const QVariant &value        , const QUaModbusValue::ValueType &type);
	
};



#endif // QUAMODBUSVALUE_H
