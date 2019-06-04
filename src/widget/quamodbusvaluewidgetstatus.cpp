#include "quamodbusvaluewidgetstatus.h"
#include "ui_quamodbusvaluewidgetstatus.h"

QUaModbusValueWidgetStatus::QUaModbusValueWidgetStatus(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QUaModbusValueWidgetStatus)
{
    ui->setupUi(this);
}

QUaModbusValueWidgetStatus::~QUaModbusValueWidgetStatus()
{
    delete ui;
}
