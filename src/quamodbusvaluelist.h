#ifndef QUAMODBUSVALUELIST_H
#define QUAMODBUSVALUELIST_H

#include <QUaFolderObject>

#include <QDomDocument>
#include <QDomElement>

class QUaModbusDataBlock;

class QUaModbusValueList : public QUaFolderObject
{
	friend class QUaModbusDataBlock;
	friend class QUaModbusValue;

    Q_OBJECT

public:
	Q_INVOKABLE explicit QUaModbusValueList(QUaServer *server);

	// UA methods

	Q_INVOKABLE QString addValue(QString strValueId);


private:
	QUaModbusDataBlock * block();

	QList<QUaModbusValue*> values();

	// XML import / export
	QDomElement toDomElement  (QDomDocument & domDoc) const;
	void        fromDomElement(QDomElement  & domElem, QString &strError);

};

#endif // QUAMODBUSVALUELIST_H


