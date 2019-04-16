#ifndef QUAMODBUSDATABLOCK_H
#define QUAMODBUSDATABLOCK_H

#include <QUaBaseObject>

class QUaModbusClient;

class QUaModbusDataBlock : public QUaBaseObject
{
    Q_OBJECT

	// UA properties
	Q_PROPERTY(QUaProperty * ModiconStartAddress READ modiconStartAddress)
	Q_PROPERTY(QUaProperty * Length              READ length             )

	// UA variables
	// TODO : the data

public:
	Q_INVOKABLE explicit QUaModbusDataBlock(QUaServer *server);

	// UA properties

	QUaProperty * modiconStartAddress();
	QUaProperty * length();

	// UA methods

	Q_INVOKABLE void remove();

private:
	QUaModbusClient * client();


};



#endif // QUAMODBUSDATABLOCK_H
