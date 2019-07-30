#include "quamodbusclientwidgettest.h"
#include "ui_quamodbusclientwidgettest.h"

#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>

#include <QUaModbusClientList>
#include <QUaModbusClient>
#include <QUaModbusTcpClient>
#include <QUaModbusRtuSerialClient>
#include <QUaModbusDataBlock>
#include <QUaModbusValue>

#include <QUaModbusClientWidget>
#include <QUaModbusDataBlockWidget>
#include <QUaModbusValueWidget>

QUaModbusClientWidgetTest::QUaModbusClientWidgetTest(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QUaModbusClientWidgetTest)
{
    ui->setupUi(this);
	m_deleting       = false;
	// instantiate widgets
	this->setupModbusWidgets();
	// add list entry point to object's folder
	QUaFolderObject * objsFolder = m_server.objectsFolder();
	auto modCliList = objsFolder->addChild<QUaModbusClientList>();
	modCliList->setDisplayName("ModbusClients");
	modCliList->setBrowseName("ModbusClients");
	// set client list into widget
	ui->widgetModbus->setClientList(modCliList);

	// change widgets
	QObject::connect(ui->widgetModbus, &QUaModbusClientTree::nodeSelectionChanged, this,
	[this](QUaNode * nodePrev, QModbusSelectType typePrev, QUaNode * nodeCurr, QModbusSelectType typeCurr) 
	{
		Q_UNUSED(nodePrev);
		Q_UNUSED(typePrev);
		// early exit
		if (m_deleting || !nodeCurr)
		{
			return;
		}
		// set up widgets for current selection
		switch (typeCurr)
		{
		case QModbusSelectType::QUaModbusClient:
			{
				auto client = dynamic_cast<QUaModbusClient*>(nodeCurr);
				this->bindClientWidget(client);
			}
			break;
		case QModbusSelectType::QUaModbusDataBlock:
			{
				auto block = dynamic_cast<QUaModbusDataBlock*>(nodeCurr);
				this->bindBlockWidget(block);
			}
			break;
		case QModbusSelectType::QUaModbusValue:
			{
				auto value = dynamic_cast<QUaModbusValue*>(nodeCurr);
				this->bindValueWidget(value);
			}
			break;
		default:
			break;
		}
	});
}

QUaModbusClientWidgetTest::~QUaModbusClientWidgetTest()
{
	m_deleting = true;
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

void QUaModbusClientWidgetTest::on_pushButtonImport_clicked()
{
	// setup error dialog just in case
	QMessageBox msgBox;
	msgBox.setWindowTitle("Error");
	msgBox.setIcon(QMessageBox::Critical);
	// read from file
	QString strLoadFile = QFileDialog::getOpenFileName(this, tr("Open File"),
		QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
		QObject::trUtf8("XML (*.xml *.txt)"));
	// validate
	if (strLoadFile.isEmpty())
	{
		return;
	}
	QFile fileConfig(strLoadFile);
	// exists
	if (!fileConfig.exists())
	{
		msgBox.setText(tr("File %1 does not exist.").arg(strLoadFile));
		msgBox.exec();
	}
	else if (fileConfig.open(QIODevice::ReadOnly))
	{
		// load config into client list
		auto modCliList = m_server.objectsFolder()->browseChild<QUaModbusClientList>();
		Q_CHECK_PTR(modCliList);
		auto strError   = modCliList->setXmlConfig(fileConfig.readAll());
		if (strError.contains("Error"))
		{
			msgBox.setText(strError);
			msgBox.exec();
		}
	}
	else
	{
		msgBox.setText(tr("File %1 could not be opened.").arg(strLoadFile));
		msgBox.exec();
	}
}

void QUaModbusClientWidgetTest::on_pushButtonExport_clicked()
{
	// select file
	QString strSaveFile = QFileDialog::getSaveFileName(this, tr("Save File"),
		QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
		tr("XML (*.xml *.txt)"));
	// ignore if empty
	if (strSaveFile.isEmpty() || strSaveFile.isNull())
	{
		return;
	}
	// save to file
	QString strSaveError;
	QFile file(strSaveFile);
	if (file.open(QIODevice::ReadWrite | QFile::Truncate))
	{
		// convert config to xml
		auto modCliList = m_server.objectsFolder()->browseChild<QUaModbusClientList>();
		Q_CHECK_PTR(modCliList);
		// get config
		QTextStream stream(&file);
		stream << modCliList->xmlConfig();
	}
	else
	{
		strSaveError = tr("Error opening file ") + strSaveFile + tr(" for write operations.");
	}
	// close file
	file.close();
	// show error dialog
	if (!strSaveError.isEmpty())
	{
		QMessageBox msgBox;
		msgBox.setWindowTitle("Error");
		msgBox.setIcon(QMessageBox::Critical);
		msgBox.setText(tr("Could not create file ") + strSaveFile + ". " + strSaveError);
		msgBox.exec();
	}
}

void QUaModbusClientWidgetTest::setupModbusWidgets()
{
	m_widgetClient = new QUaModbusClientWidget   (ui->stackedWidget);
	m_widgetBlock  = new QUaModbusDataBlockWidget(ui->stackedWidget);
	m_widgetValue  = new QUaModbusValueWidget	 (ui->stackedWidget);
	auto emptyWidget = new QWidget(ui->stackedWidget);
	// add to stack widget
	ui->stackedWidget->insertWidget(static_cast<int>(QModbusWidgets::Client ), m_widgetClient);
	ui->stackedWidget->insertWidget(static_cast<int>(QModbusWidgets::Block  ), m_widgetBlock );
	ui->stackedWidget->insertWidget(static_cast<int>(QModbusWidgets::Value  ), m_widgetValue );
	ui->stackedWidget->insertWidget(static_cast<int>(QModbusWidgets::Invalid), emptyWidget   );
	// set current to invalid
	ui->stackedWidget->setCurrentIndex(static_cast<int>(QModbusWidgets::Invalid));
}

void QUaModbusClientWidgetTest::bindClientWidget(QUaModbusClient * client)
{
	// show client widget on stack widget
	ui->stackedWidget->setCurrentIndex(static_cast<int>(QModbusWidgets::Client));
	// bind widget
	m_widgetClient->bindClient(client);
}

void QUaModbusClientWidgetTest::bindBlockWidget(QUaModbusDataBlock * block)
{
	// show block widget on stack widget
	ui->stackedWidget->setCurrentIndex(static_cast<int>(QModbusWidgets::Block));
	// bind widget
	m_widgetBlock->bindBlock(block);
}

void QUaModbusClientWidgetTest::bindValueWidget(QUaModbusValue * value)
{
	// show value widget on stack widget
	ui->stackedWidget->setCurrentIndex(static_cast<int>(QModbusWidgets::Value));
	// bind widget
	m_widgetValue->bindValue(value);
}
