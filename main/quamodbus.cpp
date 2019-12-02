#include "quamodbus.h"
#include "ui_quamodbus.h"

QUaModbus::QUaModbus(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::QUaModbus)
{
    ui->setupUi(this);
}

QUaModbus::~QUaModbus()
{
    delete ui;
}

