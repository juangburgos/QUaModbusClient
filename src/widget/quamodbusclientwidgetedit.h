#ifndef QUAMODBUSCLIENTWIDGETEDIT_H
#define QUAMODBUSCLIENTWIDGETEDIT_H

#include <QWidget>

namespace Ui {
class QUaModbusClientWidgetEdit;
}

class QUaModbusClientWidgetEdit : public QWidget
{
    Q_OBJECT

public:
    explicit QUaModbusClientWidgetEdit(QWidget *parent = nullptr);
    ~QUaModbusClientWidgetEdit();

private:
    Ui::QUaModbusClientWidgetEdit *ui;
};

#endif // QUAMODBUSCLIENTWIDGETEDIT_H
