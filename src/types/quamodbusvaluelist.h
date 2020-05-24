#ifndef QUAMODBUSVALUELIST_H
#define QUAMODBUSVALUELIST_H

#ifndef QUA_ACCESS_CONTROL
#include <QUaFolderObject>
#else
#include <QUaFolderObjectProtected>
#endif // !QUA_ACCESS_CONTROL

#include <QDomDocument>
#include <QDomElement>

class QUaModbusDataBlock;
class QUaModbusValue;

#ifndef QUA_ACCESS_CONTROL
class QUaModbusValueList : public QUaFolderObject
#else
class QUaModbusValueList : public QUaFolderObjectProtected
#endif // !QUA_ACCESS_CONTROL
{
	friend class QUaModbusDataBlock;
	friend class QUaModbusValue;

    Q_OBJECT

public:
	Q_INVOKABLE explicit QUaModbusValueList(QUaServer *server);

	// UA methods

	Q_INVOKABLE QString addValue(const QUaQualifiedName& valueId);

	Q_INVOKABLE void clear();

	// C++ API

	QList<QUaModbusValue*> values();

private:
	QUaModbusDataBlock * block();

	// XML import / export
	QDomElement toDomElement  (QDomDocument & domDoc) const;
	void        fromDomElement(QDomElement  & domElem, QQueue<QUaLog>& errorLogs);

};

#endif // QUAMODBUSVALUELIST_H


