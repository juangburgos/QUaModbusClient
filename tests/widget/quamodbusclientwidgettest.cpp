#include "quamodbusclientwidgettest.h"
#include "ui_quamodbusclientwidgettest.h"

#include <QUaModbusClientList>

QUaModbusClientWidgetTest::QUaModbusClientWidgetTest(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QUaModbusClientWidgetTest)
{
    ui->setupUi(this);
	// add list entry point to object's folder
	QUaFolderObject * objsFolder = m_server.objectsFolder();
	auto modCliList = objsFolder->addChild<QUaModbusClientList>();
	modCliList->setDisplayName("ModbusClients");
	modCliList->setBrowseName("ModbusClients");
	// set client list into widget
	ui->widgetModbus->setClientList(modCliList);
}

QUaModbusClientWidgetTest::~QUaModbusClientWidgetTest()
{
    delete ui;
}

void QUaModbusClientWidgetTest::on_pushButtonStart_clicked()
{
	m_server.isRunning() ?
		ui->pushButtonStart->setText(tr("Start")) :
		ui->pushButtonStart->setText(tr("Stop"));
	m_server.isRunning() ?
		m_server.stop() :
		m_server.start();
}
