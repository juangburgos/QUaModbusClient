#include "quamodbusclientdialog.h"
#include "ui_quamodbusclientdialog.h"

QUaModbusClientDialog::QUaModbusClientDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QUaModbusClientDialog)
{
    ui->setupUi(this);
}

QUaModbusClientDialog::~QUaModbusClientDialog()
{
    delete ui;
}

QWidget * QUaModbusClientDialog::widget() const
{
    // find the widget and return a reference to it
    return this->findChild<QWidget*>("diagwidget");
}

void QUaModbusClientDialog::setWidget(QWidget * w)
{
    // check pointer
    Q_CHECK_PTR(w);
    if (!w)
    {
        return;
    }
    // check if has widget already
    Q_ASSERT(!this->widget());
    if (this->widget())
    {
        return;
    }
    // take ownership
    w->setParent(this);
    // set object name in order to be able to retrieve later
    w->setObjectName("diagwidget");
    // put the widget in the layout
    ui->verticalLayout->insertWidget(0, w);
    ui->verticalLayout->setSizeConstraint(QLayout::SetFixedSize);
}
