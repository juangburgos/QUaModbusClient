#ifndef QUAMODBUSDATABLOCK_H
#define QUAMODBUSDATABLOCK_H

#include <QModbusDataUnit>
#include <QModbusReply>

#include <QUaBaseObject>

class QUaModbusClient;
class QUaModbusDataBlockList;

class QUaModbusDataBlock : public QUaBaseObject
{
	friend class QUaModbusDataBlockList;

    Q_OBJECT

	// UA properties
	Q_PROPERTY(QUaProperty * Type         READ type        )
	Q_PROPERTY(QUaProperty * Address      READ address     )
	Q_PROPERTY(QUaProperty * Size         READ size        )
	Q_PROPERTY(QUaProperty * SamplingTime READ samplingTime)

	// UA variables
	Q_PROPERTY(QUaBaseDataVariable * Data      READ data        )
	Q_PROPERTY(QUaBaseDataVariable * LastError READ lastError   )

public:
	Q_INVOKABLE explicit QUaModbusDataBlock(QUaServer *server);

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

	QUaProperty * type        ();
	QUaProperty * address     ();
	QUaProperty * size        ();
	QUaProperty * samplingTime();

	// UA variables

	QUaBaseDataVariable * data();
	QUaBaseDataVariable * lastError();

	// UA methods

	Q_INVOKABLE void remove();

private slots:
	void on_typeChanged        (const QVariant &value);
	void on_addressChanged     (const QVariant &value);
	void on_sizeChanged        (const QVariant &value);
	void on_samplingTimeChanged(const QVariant &value);

private:
	int m_loopHandle;
	QModbusReply * m_reply;
	QModbusDataUnit m_modbusDataUnit; // NOTE : only modify and access in thread
	QUaModbusClient * client();
	void startLoop();

	static quint32 m_minSamplingTime;
};



#endif // QUAMODBUSDATABLOCK_H
