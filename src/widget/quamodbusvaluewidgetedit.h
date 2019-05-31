#ifndef QUAMODBUSVALUEWIDGETEDIT_H
#define QUAMODBUSVALUEWIDGETEDIT_H

#include <QWidget>

namespace Ui {
class QUaModbusValueWidgetEdit;
}

class QUaModbusValueWidgetEdit : public QWidget
{
    Q_OBJECT

public:
    explicit QUaModbusValueWidgetEdit(QWidget *parent = nullptr);
    ~QUaModbusValueWidgetEdit();

private:
    Ui::QUaModbusValueWidgetEdit *ui;
};

#endif // QUAMODBUSVALUEWIDGETEDIT_H
