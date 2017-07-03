#pragma once

#include <QtWidgets/QtWidgets>

class IPUI;

class QRGBLabel :public QLabel
{
	Q_OBJECT

public:
	QRGBLabel(QWidget *parent = 0);
	virtual ~QRGBLabel();

	void SetManual(bool bManual);

	void Reset();

protected:
	virtual void mousePressEvent(QMouseEvent * evt);
	virtual void paintEvent(QPaintEvent *evt);

private:
	IPUI* m_pIPUI;

	QPoint m_ptSelected;

	bool m_bManual;

	QPixmap m_pixCircle;
	QPixmap m_pixScaled;

};
