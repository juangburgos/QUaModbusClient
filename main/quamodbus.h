#ifndef QUAMODBUS_H
#define QUAMODBUS_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class QUaModbus; }
QT_END_NAMESPACE

class QUaModbus : public QMainWindow
{
    Q_OBJECT

public:
    QUaModbus(QWidget *parent = nullptr);
    ~QUaModbus();

private:
    Ui::QUaModbus *ui;
};
#endif // QUAMODBUS_H
