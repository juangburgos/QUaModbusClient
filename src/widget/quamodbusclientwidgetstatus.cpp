#include "quamodbusclientwidgetstatus.h"
#include "ui_quamodbusclientwidgetstatus.h"

QUaModbusClientWidgetStatus::QUaModbusClientWidgetStatus(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QUaModbusClientWidgetStatus)
{
    ui->setupUi(this);
}

QUaModbusClientWidgetStatus::~QUaModbusClientWidgetStatus()
{
    delete ui;
}

void QUaModbusClientWidgetStatus::setStatus(const QModbusError & status)
{
	auto metaError = QMetaEnum::fromType<QModbusError>();
	auto strError  = QString(metaError.valueToKey(status));
	ui->lineEditStatus->setText(strError);
}

void QUaModbusClientWidgetStatus::setState(const QModbusState & state)
{
	auto metaState = QMetaEnum::fromType<QModbusState>();
	auto strState  = QString(metaState.valueToKey(state));
	ui->lineEditState->setText(strState);
}
