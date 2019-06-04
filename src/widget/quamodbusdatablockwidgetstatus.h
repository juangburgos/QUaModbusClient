#ifndef QUAMODBUSDATABLOCKWIDGETSTATUS_H
#define QUAMODBUSDATABLOCKWIDGETSTATUS_H

#include <QWidget>

namespace Ui {
class QUaModbusDataBlockWidgetStatus;
}

class QUaModbusDataBlockWidgetStatus : public QWidget
{
    Q_OBJECT

public:
    explicit QUaModbusDataBlockWidgetStatus(QWidget *parent = nullptr);
    ~QUaModbusDataBlockWidgetStatus();

private:
    Ui::QUaModbusDataBlockWidgetStatus *ui;
};

#endif // QUAMODBUSDATABLOCKWIDGETSTATUS_H
