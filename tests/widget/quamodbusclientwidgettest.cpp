#include "quamodbusclientwidgettest.h"
#include "ui_quamodbusclientwidgettest.h"

#include <QUaModbusClientList>
#include <QUaModbusClient>
#include <QUaModbusDataBlock>
#include <QUaModbusValue>

#include <QUaModbusClientWidgetEdit>
#include <QUaModbusDataBlockWidgetEdit>
#include <QUaModbusValueWidgetEdit>

QUaModbusClientWidgetTest::QUaModbusClientWidgetTest(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QUaModbusClientWidgetTest)
{
    ui->setupUi(this);
	m_pWidgetEdit = nullptr;
	m_typeModbusCurr = QModbusSelectType::Invalid;
	// add list entry point to object's folder
	QUaFolderObject * objsFolder = m_server.objectsFolder();
	auto modCliList = objsFolder->addChild<QUaModbusClientList>();
	modCliList->setDisplayName("ModbusClients");
	modCliList->setBrowseName("ModbusClients");
	// set client list into widget
	ui->widgetModbus->setClientList(modCliList);

	// change widgets
	QObject::connect(ui->widgetModbus, &QUaModbusClientWidget::nodeSelectionChanged, this,
		[this](QUaNode * nodePrev, QModbusSelectType typePrev, QUaNode * nodeCurr, QModbusSelectType typeCurr) 
	{
		// delete old widget if necessary
		if (m_pWidgetEdit && m_typeModbusCurr != typeCurr)
		{
			delete m_pWidgetEdit;
			m_pWidgetEdit = nullptr;
		}
		// set up widgets for current selection
		switch (typeCurr)
		{
		case QModbusSelectType::QUaModbusClient:
			{
				auto client = dynamic_cast<QUaModbusClient*>(nodeCurr);
				this->bindClientWidgetEdit(client);
			}
			break;
		case QModbusSelectType::QUaModbusDataBlock:
			{
				auto block = dynamic_cast<QUaModbusDataBlock*>(nodeCurr);
				this->bindBlockWidgetEdit(block);
			}
			break;
		case QModbusSelectType::QUaModbusValue:
			{
				auto value = dynamic_cast<QUaModbusValue*>(nodeCurr);
				this->bindValueWidgetEdit(value);
			}
			break;
		default:
			if (m_pWidgetEdit)
			{
				delete m_pWidgetEdit;
			}
			m_pWidgetEdit = nullptr;
			break;
		}
	});

}

QUaModbusClientWidgetTest::~QUaModbusClientWidgetTest()
{
    delete ui;
}

void QUaModbusClientWidgetTest::bindClientWidgetEdit(QUaModbusClient * client)
{
	// create widget if necessary
	QUaModbusClientWidgetEdit * widget = nullptr;
	if (!m_pWidgetEdit)
	{
		widget = new QUaModbusClientWidgetEdit(this);
		m_pWidgetEdit = widget;
		ui->verticalLayoutConfig->insertWidget(0, widget);
	}
	else
	{
		widget = dynamic_cast<QUaModbusClientWidgetEdit*>(m_pWidgetEdit);
	}
	Q_CHECK_PTR(widget);
	// bind

}

void QUaModbusClientWidgetTest::bindBlockWidgetEdit(QUaModbusDataBlock * block)
{
	// create widget if necessary
	QUaModbusDataBlockWidgetEdit * widget = nullptr;
	if (!m_pWidgetEdit)
	{
		widget = new QUaModbusDataBlockWidgetEdit(this);
		m_pWidgetEdit = widget;
		ui->verticalLayoutConfig->insertWidget(0, widget);
	}
	else
	{
		widget = dynamic_cast<QUaModbusDataBlockWidgetEdit*>(m_pWidgetEdit);
	}
	Q_CHECK_PTR(widget);
	// bind

}

void QUaModbusClientWidgetTest::bindValueWidgetEdit(QUaModbusValue * value)
{
	// create widget if necessary
	QUaModbusValueWidgetEdit * widget = nullptr;
	if (!m_pWidgetEdit)
	{
		widget = new QUaModbusValueWidgetEdit(this);
		m_pWidgetEdit = widget;
		ui->verticalLayoutConfig->insertWidget(0, widget);
	}
	else
	{
		widget = dynamic_cast<QUaModbusValueWidgetEdit*>(m_pWidgetEdit);
	}
	Q_CHECK_PTR(widget);
	// bind

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
