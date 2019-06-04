#ifndef QUAMODBUSVALUEWIDGETSTATUS_H
#define QUAMODBUSVALUEWIDGETSTATUS_H

#include <QWidget>

namespace Ui {
class QUaModbusValueWidgetStatus;
}

class QUaModbusValueWidgetStatus : public QWidget
{
    Q_OBJECT

public:
    explicit QUaModbusValueWidgetStatus(QWidget *parent = nullptr);
    ~QUaModbusValueWidgetStatus();

private:
    Ui::QUaModbusValueWidgetStatus *ui;
};

#endif // QUAMODBUSVALUEWIDGETSTATUS_H
