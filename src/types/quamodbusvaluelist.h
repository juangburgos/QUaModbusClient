#ifndef QUAMODBUSVALUELIST_H
#define QUAMODBUSVALUELIST_H

#include <QUaFolderObject>

#include <QDomDocument>
#include <QDomElement>

class QUaModbusDataBlock;
class QUaModbusValue;

class QUaModbusValueList : public QUaFolderObject
{
	friend class QUaModbusDataBlock;
	friend class QUaModbusValue;

    Q_OBJECT

public:
	Q_INVOKABLE explicit QUaModbusValueList(QUaServer *server);

	// UA methods

	Q_INVOKABLE QString addValue(QString strValueId);

	Q_INVOKABLE void clear();

	// C++ API

	QList<QUaModbusValue*> values();

private:
	QUaModbusDataBlock * block();

	// XML import / export
	QDomElement toDomElement  (QDomDocument & domDoc) const;
	void        fromDomElement(QDomElement  & domElem, QString &strError);

};

#endif // QUAMODBUSVALUELIST_H


