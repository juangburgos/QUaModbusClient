#include "quamodbusvaluewidgetedit.h"
#include "ui_quamodbusvaluewidgetedit.h"

QUaModbusValueWidgetEdit::QUaModbusValueWidgetEdit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QUaModbusValueWidgetEdit)
{
    ui->setupUi(this);
}

QUaModbusValueWidgetEdit::~QUaModbusValueWidgetEdit()
{
    delete ui;
}
