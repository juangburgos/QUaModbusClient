#ifndef QUAMODBUSCOMMONWIDGETS_H
#define QUAMODBUSCOMMONWIDGETS_H

#include <QWidget>
#include <QComboBox>

class QUaModbusReadOnlyComboBox : public QComboBox
{
	Q_OBJECT

public:
	explicit QUaModbusReadOnlyComboBox(QWidget *parent = Q_NULLPTR);

	bool isReadOnly() const;
	void setReadOnly(const bool &bReadOnly);

protected:
	void mousePressEvent(QMouseEvent *e) override;
	void keyPressEvent(QKeyEvent   *e) override;
	void wheelEvent(QWheelEvent *e) override;

private:
	bool m_bReadOnly;
};

#endif // QUAMODBUSCOMMONWIDGETS_H