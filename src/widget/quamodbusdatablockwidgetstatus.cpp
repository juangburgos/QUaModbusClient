#include "quamodbusdatablockwidgetstatus.h"
#include "ui_quamodbusdatablockwidgetstatus.h"

QUaModbusDataBlockWidgetStatus::QUaModbusDataBlockWidgetStatus(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QUaModbusDataBlockWidgetStatus)
{
    ui->setupUi(this);
}

QUaModbusDataBlockWidgetStatus::~QUaModbusDataBlockWidgetStatus()
{
    delete ui;
}
