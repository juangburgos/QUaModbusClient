#include "quamodbusclientwidgetedit.h"
#include "ui_quamodbusclientwidgetedit.h"

QUaModbusClientWidgetEdit::QUaModbusClientWidgetEdit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QUaModbusClientWidgetEdit)
{
    ui->setupUi(this);
}

QUaModbusClientWidgetEdit::~QUaModbusClientWidgetEdit()
{
    delete ui;
}
