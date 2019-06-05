#ifndef QUAMODBUSCLIENTWIDGETSTATUS_H
#define QUAMODBUSCLIENTWIDGETSTATUS_H

#include <QWidget>
#include <QUaModbusClient>

namespace Ui {
class QUaModbusClientWidgetStatus;
}

class QUaModbusClientWidgetStatus : public QWidget
{
    Q_OBJECT

public:
    explicit QUaModbusClientWidgetStatus(QWidget *parent = nullptr);
    ~QUaModbusClientWidgetStatus();

	void setStatus(const QModbusError &status);

	void setState(const QModbusState &state);

private:
    Ui::QUaModbusClientWidgetStatus *ui;
};

#endif // QUAMODBUSCLIENTWIDGETSTATUS_H
