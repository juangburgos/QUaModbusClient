#ifndef QUAMODBUSDATABLOCK_H
#define QUAMODBUSDATABLOCK_H

#include <QModbusDataUnit>
#include <QModbusReply>

#include <QUaBaseObject>

#include <QDomDocument>
#include <QDomElement>

class QUaModbusClient;
class QUaModbusDataBlockList;
class QUaModbusValue;

#include "quamodbusvaluelist.h"

class QUaModbusDataBlock : public QUaBaseObject
{
	friend class QUaModbusDataBlockList;
	friend class QUaModbusValue;

    Q_OBJECT

	// UA properties
	Q_PROPERTY(QUaProperty * Type         READ type        )
	Q_PROPERTY(QUaProperty * Address      READ address     )
	Q_PROPERTY(QUaProperty * Size         READ size        )
	Q_PROPERTY(QUaProperty * SamplingTime READ samplingTime)

	// UA variables
	Q_PROPERTY(QUaBaseDataVariable * Data      READ data     )
	Q_PROPERTY(QUaBaseDataVariable * LastError READ lastError)

	// UA objects
	Q_PROPERTY(QUaModbusValueList * Values READ values)

public:
	Q_INVOKABLE explicit QUaModbusDataBlock(QUaServer *server);
	~QUaModbusDataBlock();

	enum RegisterType 
	{
		Invalid          = QModbusDataUnit::RegisterType::Invalid         ,
		DiscreteInputs   = QModbusDataUnit::RegisterType::DiscreteInputs  ,
		Coils            = QModbusDataUnit::RegisterType::Coils           ,
		InputRegisters   = QModbusDataUnit::RegisterType::InputRegisters  ,
		HoldingRegisters = QModbusDataUnit::RegisterType::HoldingRegisters
	};
	Q_ENUM(RegisterType)

	// UA properties

	QUaProperty * type        () const;
	QUaProperty * address     () const;
	QUaProperty * size        () const;
	QUaProperty * samplingTime() const;

	// UA variables

	QUaBaseDataVariable * data() const;
	QUaBaseDataVariable * lastError() const;

	// UA objects

	QUaModbusValueList * values() const;

	// UA methods

	Q_INVOKABLE void remove();

signals:
	// to safely update error in ua server thread
	void updateLastError(const QModbusDevice::Error &error);

private slots:
	void on_typeChanged        (const QVariant &value);
	void on_addressChanged     (const QVariant &value);
	void on_sizeChanged        (const QVariant &value);
	void on_samplingTimeChanged(const QVariant &value);
	void on_dataChanged        (const QVariant &value);
	void on_updateLastError    (const QModbusDevice::Error &error);

private:
	int m_loopHandle;
	QModbusReply  * m_replyRead;
	QModbusDataUnit m_modbusDataUnit; // NOTE : only modify and access in thread
	QUaModbusClient * client();
	void startLoop();

	// XML import / export
	QDomElement toDomElement  (QDomDocument & domDoc) const;
	void        fromDomElement(QDomElement  & domElem, QString &strError);

	static quint32 m_minSamplingTime;
	static QVector<quint16> variantToInt16Vect(const QVariant &value);
};

#endif // QUAMODBUSDATABLOCK_H
