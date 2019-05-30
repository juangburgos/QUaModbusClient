#ifndef QUAMODBUSCLIENTDIALOG_H
#define QUAMODBUSCLIENTDIALOG_H

#include <QDialog>

namespace Ui {
class QUaModbusClientDialog;
}

class QUaModbusClientDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QUaModbusClientDialog(QWidget *parent = nullptr);
    ~QUaModbusClientDialog();

    QWidget * widget() const;
    void      setWidget(QWidget * w);

private:
    Ui::QUaModbusClientDialog *ui;
};

#endif // QUAMODBUSCLIENTDIALOG_H
