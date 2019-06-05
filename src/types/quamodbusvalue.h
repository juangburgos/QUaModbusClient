#ifndef QUAMODBUSVALUE_H
#define QUAMODBUSVALUE_H

#include <QModbusDataUnit>
#include <QModbusReply>

#include <QUaBaseObject>

#include <QDomDocument>
#include <QDomElement>

class QUaModbusDataBlock;
class QUaModbusValueList;

typedef QModbusDevice::Error QModbusError;

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
	typedef QUaModbusValue::ValueType QModbusValueType;

	// UA properties

	QUaProperty * type() const;
	QUaProperty * registersUsed() const;
	QUaProperty * addressOffset() const;

	// UA variables

	QUaBaseDataVariable * value() const;
	QUaBaseDataVariable * lastError() const;

	// UA methods

	Q_INVOKABLE void remove();

	// C++ API
	QModbusValueType getType() const;
	void             setType(const QModbusValueType &type);

	quint16 getRegistersUsed() const;

	int  getAddressOffset() const;
	void setAddressOffset(const int &addressOffset);

	QVariant getValue() const;
	void     setValue(const QVariant &value);

	QModbusError getLastError() const;
	void         setLastError(const QModbusError &error);

	QUaModbusDataBlock * block() const;

	static int              typeBlockSize(const QModbusValueType &type);
	static QMetaType::Type  typeToMeta   (const QModbusValueType &type);
	static QVariant         blockToValue (const QVector<quint16> &block, const QModbusValueType &type);
	static QVector<quint16> valueToBlock (const QVariant         &value, const QModbusValueType &type);

signals:
	// C++ API
	void typeChanged         (const QModbusValueType &type         );
	void registersUsedChanged(const quint16          &registersUsed);
	void addressOffsetChanged(const int              &addressOffset);
	void valueChanged        (const QVariant         &value        );
	void lastErrorChanged    (const QModbusError     &error        );

private slots:
	void on_typeChanged         (const QVariant &value);
	void on_addressOffsetChanged(const QVariant &value);
	void on_valueChanged        (const QVariant &value);

private:
	void setValue(const QVector<quint16> &block, const QModbusError &blockError);

	// XML import / export
	QDomElement toDomElement  (QDomDocument & domDoc) const;
	void        fromDomElement(QDomElement  & domElem, QString &strError);
	
};

typedef QUaModbusValue::ValueType QModbusValueType;

#endif // QUAMODBUSVALUE_H
